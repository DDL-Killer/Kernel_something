## 启动my_opengauss

`docker start my_opengauss`

## 启动docker模型

`sudo docker exec -it my_opengauss bash `

## 进入docker里的用户

`su - omm`

## 进入数据库

`gsql -d postgres -p 5432 -U omm -W "OpenGauss@123"`

