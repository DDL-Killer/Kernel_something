# Git从入门到使用

## 全局配置

```bash
# 替换为你的 GitHub 用户名
git config --global user.name "Your Name"

# 替换为你的 GitHub 注册邮箱
git config --global user.email "your_email@example.com"
```

## 本地创建

```bash
mkdir 你创建的文件夹名称
cd 你创建的文件夹名称
git init
```

## 提交到暂存区

```bash
# 把当前目录下所有修改加入暂存区 (就像把货物搬到发货台上)
git add .

# 把暂存区的内容提交到版本库 (就像贴上封条发车，并写上备注)
git commit -m "first commit: create readme"
```

## 上传到云端

```bash
# 重命名分支
git branch -M main

# 添加远程仓库地址
# 注意：把下面的网址换成你刚才在 GitHub 页面上看到的地址
# 推荐使用 SSH 地址 (git@github.com:...) 如果你配了 SSH Key
# 如果没配 SSH，就用 HTTPS 地址 (https://github.com/...)
git remote add origin https://github.com/你的用户名/learning-rust.git

# 如果出现"错误：远程 origin 已经存在"
# 1. 要么直接修改现有的origin地址
git remote set-url origin git@github.com:你的具体地址
# 2. 要么删除原有的地址重新设置
git remote remove origin
git remote add origin git@github.com:你的具体地址

# push到远程仓库去
git push -u origin main
# 如果出现报错"致命错误：当前分支 main 没有对应的上游分支"
git push --set-upstream origin main

# 之后继续push
git push
```

