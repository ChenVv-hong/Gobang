### Gobang 联机五子棋

------

#### 项目简介

初步巩固linux网络编程，重心偏向于服务端开发，客户端进行简单的快速开发提供一个交互界面。

##### 客户端

通过QT实现客户端的编写，提供友好的人机交互界面。

- QT的基础界面编程
- QT的画笔工具
- QT的基础socket编程

##### 服务端

Linux下使用c++编写的并发服务器，为客户端提供服务。

- 使用reactor事件处理模式
- 使用epoll多路IO复用技术,实现并发，采用 LT+LT epoll模式下的阻塞IO
- 通过线程池中工作线程处理IO事件和数据

#### 功能介绍

- 人机对战功能
- 匹配对战，多个玩家同时匹配
- 创建房间
- 加入房间
- 棋局内能够悔棋，认输，求和和消息发送