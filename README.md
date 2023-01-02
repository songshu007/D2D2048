# D2D-2048



C++ 版2048，使用`Direct2D`绘制



![Screenshot](./Screenshot.png)



## 项目结构



```
├─D2D2048_v3.sln	# 主工程文件
└─D2D2048_v3
	├─main.cpp		# 游戏入口
    ├─include		# 依赖的其它库文件
    │  ├─GameEngine	# 用 Direct2d 封装的简单的游戏引擎
    │  ├─httplib	# httplib 库，通过 http-get 请求远程获取一些配置、检查更新 相关代码在 src/NetWorkUpdata.cpp
    │  └─nlohmann	# json 库，用来解析上面 http-get 请求的 json 数据，因为配置是用 json 写的
    │
    └─src			# 游戏一些其他的文件
       ├─Board.cpp	# 棋盘类
       └─NetWorkUpdata.cpp	# 用来获取远程配置的类 http-get
```



## 日志



## v4.0

- 添加了远程后台用来检测更新，棋盘的下雪和背景颜色现在可以远程自定义
- 砍了许多菜单项，以后会添加回来，提示：按“H”键可以看到按键设置
- 重构整个棋盘类
- 图标被我搞没了，以后再加回来
- 重构GameEngine，现在GameEngine是单线程的，多线程现在有一些问题

## v2.5

优化动画效果

## v2.4

新增多种缓动动画，动画速度调节

## v2.3

修复了在高DPI设备上界面缩放的问题

## v2.1

修复了稳定性问题