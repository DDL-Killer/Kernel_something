

# 核心概念

## opengauss

1. 作为在大型服务器上使用的极其稳定设计的企业级数据库,编译环境绑定在CentOS或openEuler求稳的系统上,arch过于激进底层库比要求的版本高几代
2. 数据库软件有"侵入性"

3. 使用docker把openEuler操作系统环境 配置好的底层依赖以及openGauss本身打包成为了一个静态的"集装箱"(镜像)

## 安装

1. ```bash
   sudo pacman -Syu docker
   sudo systemctl enable --now docker.service
   ```

2. 创建启动openGauss

   ```bash
   sudo docker run -d \
     --name my_opengauss_1 \
     --privileged=true \
     -v /home//:/var/lib/ \
     -v /sys/fs/cgroup:/sys/fs/cgroup:ro \
     -p 5432:5432 \
     -e GS_PASSWORD="OpenGauss@123" \
     enmotech/opengauss:latest
   ```
   
   解析:
   
   **`sudo docker run -d \`**
   
   - `run`：基于镜像创建一个新的容器实例并启动它。
   - `-d` (Detach)：后台运行模式。相当于在终端命令最后加了一个 `&`，让这个微型系统在后台默默运转，而不阻塞你当前的命令行窗口。
   
   **`--name my_opengauss \`**
   
   - 命名空间标识。给这个运行起来的容器进程组打上一个人类可读的标签，方便后续用 `docker stop my_opengauss` 来控制它。
   
   **`--privileged=true \`**
   
   - **破壁指令**
     - 默认情况下，Docker 出于安全考虑，会剥夺容器进程的很多 Linux Capabilities（比如不能挂载文件系统、不能修改网络栈）。但 openGauss 内核需要进行深度系统调用。开启特权模式，相当于告诉宿主机的 Linux 内核：“信任这个容器，给它类似 root 的底层操作权限”。
   
   **`-v /sys/fs/cgroup:/sys/fs/cgroup:ro \`**
   
   - **Volume 挂载映射**
     - openGauss 内置了工作负载管理器（WLM），它需要读取系统的 `cgroup`（控制组）树来监控和分配 CPU/内存资源。
   - `ro` (Read-Only)：保护机制
     - 把宿主机的 `/sys/fs/cgroup` 目录以**只读**的方式映射给容器。这样既骗过了数据库的启动检查，又防止它失控把宿主机的资源隔离树给改乱了。
   
   **`-p 5432:5432 \`**
   
   - **网络端口转发（Port Forwarding）**
     - 前面的 `5432` 是你 Arch 宿主机的端口，后面的 `5432` 是容器内部数据库监听的端口。有了这层映射，你的外部代码（比如你自己写的 C 程序）就可以通过连接 `127.0.0.1:5432`，顺畅地把网络包打进隔离的容器内部。
   
   **`-e GS_PASSWORD="OpenGauss@123" \`**
   
   - **注入环境变量（Environment Variable）**
     - 容器启动脚本在执行第一步初始化（`gs_initdb`）时，会去读取这个环境变量，把它作为数据库最高管理员（超级用户）的初始开机密码。
   
   **`enmotech/opengauss:latest`**
   
   - **镜像坐标**
     - 告诉 Docker 引擎去云端拉取恩墨学院维护的、标签为 `latest`（最新版）的 openGauss 镜像包
   
   

## 底层逻辑

1. arch宿主机
2. 容器内的上帝(隔离的微型OS)
   1. 角色:容器的root
   2. 状态:`docker exec -it my_opengauss bash `面对的是基于openEuler/CentOS打包的独立系统环境

3. 受限的底层(OS级数据库用户)
   1. 角色:容器内的普通用户omm
   2. 状态:`su - omm`

4. 数据库内核控制器
   1. 角色:数据库系统的超级管理员(omm)
   2. 状态:`gsql -d postgres -p 5432 -U omm -W "OpenGauss@123"`进入,使用标准SQL或PL/pgSQL与数据库引擎对话

## 白盒实验

1. 验证表的存在

   ```SQL
   -- 1. 创建一张测试表
   CREATE TABLE linux_kernel (
       id INT PRIMARY KEY,
       version VARCHAR(50)
   );
   
   -- 2. 查询系统表 pg_class，揪出它的底层物理文件名 (relfilenode)
   SELECT oid, relname, relfilenode FROM pg_class WHERE relname = 'linux_kernel';
   ```

2. 假设查到的表是:

   | oid          | relname      | relfilenode |
   | ------------ | ------------ | ----------- |
   | 16389(1 row) | linux_kernel | 16389       |

   返回上一步`\q`后在第三层输入`find /var/lib/opengauss/data/base -name "16389" -exec stat {} \;`

   

3. 发现查到的表是0字节

4. 回到第四层,给它插入一段数据

   ```SQL
   INSERT INTO linux_kernel VALUES (1, '6.6 LTS', TRUE);
   ```

5. 退出回到第三层,再次查询

   ```bash
   # 找到文件并查看基础信息
   find /var/lib/opengauss/data/base -name "16389" -exec ls -lh {} \;
   
   # 查看这个文件在 Linux 操作系统里的详细元数据 (Inode)
   find /var/lib/opengauss/data/base -name "16389" -exec stat {} \;
   ```

6. 发现:

   ```bash
     File: /var/lib/opengauss/data/base/14819/16389
     Size: 8192            Blocks: 16         IO Block: 4096   regular file
   Device: 3eh/62d Inode: 3765047     Links: 1
   Access: (0600/-rw-------)  Uid: (   70/     omm)   Gid: (   70/     omm)
   Access: 2026-03-04 00:28:07.786329864 +0000
   Modify: 2026-03-04 00:34:06.578649366 +0000
   Change: 2026-03-04 00:34:06.578649366 +0000
    Birth: 2026-03-04 00:28:07.786329864 +0000
   
   -rw------- 1 omm omm 8.0K Mar  4 00:34 /var/lib/opengauss/data/base/14819/16389
   ```

   表示:数据库向操作系统请求分配了一个标准的8KB内存页并且刷入了磁盘

