# intOJ-judger
[intOJ](https://github.com/intoj/intoj) 的评测端，使用 Qt5 开发。

目前只是一个可以实现基本评测功能的简易版本，更多功能还在开发中。

# TODO LIST
- Special Judge
- 比赛评测
- 自定义数据规则
- Extra Test
- 编译时的空间限制

# 第三方代码

- 连接 redis 使用 [hiredis](https://github.com/redis/hiredis)
- 连接 mariadb 使用 libmysqlclient
- 评测 runner 参考 [bashu-onlinejudge](https://github.com/z4yx/bashu-onlinejudge)
- 评测沙箱来自 [libsablebox](https://github.com/yangzhixuan/libsablebox)
- 文件比较参考 [lemon](https://github.com/Dust1404/Project_LemonPlus)

# 安装方法

以下部分命令需要使用 root 权限。

首先安装依赖项：
```
apt update
apt install qt5-default g++ gcc make libmysqlclient-dev
```

下载源码，先安装依赖项 `hiredis`：
```
cd hiredis
make
make install
ldconfig
```

编译沙箱以及评测 runner：
```
cd resources
make
```

编译评测端：
```
qmake
make
```

# 部署方法

按照上述命令安装好评测端后，先修改 `config.json` 文件，注意其中 `dataDir` 和 `tempDir` 后不要加多余的 `/`。

`language` 中的 `name` 与网页端一致。编译命令 `command` 中一定要加 `-static`，否则会造成评测时判断程序 Runtime Error。

`spjTimeLimit` 和 `compileTimeList` 都是以 `ms` 作为单位。

接着在终端中输入
```
./intOJ-judger
```

即可运行评测端。

如果评测时需要开栈，请在终端中运行
```
ulimit -s unlimited
./intOJ-judger
```