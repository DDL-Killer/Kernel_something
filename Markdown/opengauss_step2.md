# 🚀 学生成绩管理系统开发日志：第二阶段（含测试程序详解）

**日期：** 2026-03-09 **环境：** Arch Linux / Docker / openGauss 5.0.0 **核心目标：** 打通宿主机 C 程序与容器内数据库的通信隧道。

------

## 1. 环境准备：Arch Linux 侧

- **安装库文件**：`sudo pacman -S postgresql-libs`。
- **头文件位置**：`/usr/include/postgresql/libpq-fe.h`。
- **编译命令**：`gcc connect_db.c -o connect_db -I/usr/include/postgresql -lpq`。

------

## 2. 核心测试程序：`connect_db.c`

这个程序是我们整个系统的“敲门砖”，它验证了网络、权限和驱动是否全部就绪。

```c
#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h> // libpq 的核心头文件，fe 代表 Front-End (前端接口)

int main() {
    // 1. 配置连接字符串 (Connection String)
    // 包含了地址(host)、端口(port)、库名(dbname)、用户(user)和密码(password)
    const char *conninfo = "host=127.0.0.1 port=5432 dbname=postgres user=meowzart password=meowzart@123";

    // 2. 尝试建立连接 (非阻塞式建立连接)
    // PQconnectdb 会在内存中创建一个 PGconn 结构体，记录这条连接的所有信息
    PGconn *conn = PQconnectdb(conninfo);

    // 3. 状态检查 (Health Check)
    // PQstatus(conn) 如果不等于 CONNECTION_OK，说明拨号失败了
    if (PQstatus(conn) != CONNECTION_OK) {
        // PQerrorMessage 会提取出数据库返回的具体报错信息（如：密码错误、用户不存在等）
        fprintf(stderr, "连接失败: %s\n", PQerrorMessage(conn));
        
        // PQfinish 相当于挂断电话，必须调用它来释放内存资源
        PQfinish(conn);
        return 1;
    }

    // 4. 成功反馈
    printf("恭喜！Meowzart，你已成功连接到 openGauss 数据库！\n");

    // 5. 优雅退出
    PQfinish(conn);
    return 0;
}
```

------

## 3. 测试程序深度讲解

### 3.1 为什么需要 `PGconn`？

在 C 语言中，`PGconn` 是一个**句柄（Handle）**。你可以把它想象成一个遥控器。一旦连接成功，这个遥控器就代表了那条通往 Docker 的 TCP 隧道。之后所有的 `SELECT`、`INSERT` 动作，都需要把这个“遥控器”传给相应的函数。

### 3.2 报错信息的价值

你在测试中遇到了两个至关重要的报错：

1. **`Forbid remote connection with initial user`**：
   - **含义**：这是 openGauss 的安全策略，禁止 `omm`（超级用户）远程登录。
   - **教训**：在生产环境中，必须遵循**最小权限原则**，为业务程序创建独立的低权限账号。
2. **`invalid connection option "meowzart"`**：
   - **含义**：连接字符串格式错误。
   - **教训**：C 语言的字符串解析非常死板。键值对（如 `user=xxx`）必须严格遵守，中间不能有多余的字符。

------

## 4. 通信原理解析

- **进程间通信**：C 程序在宿主机进程运行，openGauss 在 Docker 容器进程运行。
- **网络中转**：Arch Linux 的回环地址（127.0.0.1）充当了中转站，Docker 引擎通过 NAT 技术将流量导入容器。

------

## 5. 下阶段预告：持久化交互

既然 `connect_db.c` 已经跑通，证明了“电话线”是顺畅的。第三阶段我们将不再让程序执行完就退出，而是使用 `while(1)` 循环，让它变成一个真正的**管理控制台**。