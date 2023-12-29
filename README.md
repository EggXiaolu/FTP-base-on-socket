## 功能
1. put < filename > 上传文件 
2. get < filename > 下载文件
3. mput < ext > 上传指定后缀文件
4. mget < ext > 下载指定后缀文件
5. lremove < filename > 删除本地文件
6. rremove < filename > 删除远程文件

## 使用
编译
```bash
cd ./Client/
make
cd ../Server/
make
```

启动服务器
```bash
./Server/server
```

启动客户端
```bash
./Client/client
```

