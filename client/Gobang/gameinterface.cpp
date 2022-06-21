#include "gameinterface.h"
#include "ui_gameinterface.h"

GameInterface::GameInterface(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::GameInterface)
{
    ui->setupUi(this);
    gl = new GameLogic();
//    this->mycolor = NO_PIECE;
    this->status = WAIT_START;
    this->type = 0;
    //设置窗口属性调用 close()时 调用析构函数
    setAttribute(Qt::WA_DeleteOnClose);
    //设置窗口为应用程序 打开此窗口时 阻塞其他窗口
    setWindowModality(Qt::ApplicationModal);
    //信号槽连接
    connect(this->ui->pushButton, SIGNAL(clicked()), this, SLOT(enter()));
    connect(this->ui->pushButton_back, SIGNAL(clicked()), this, SLOT(pressButtonBack()));
    connect(this->ui->pushButton_draw, SIGNAL(clicked()), this, SLOT(pressButtonDraw()));
    connect(this->ui->pushButton_surrend, SIGNAL(clicked()), this, SLOT(pressButtonSurrend()));
    connect(this->ui->lineEdit, SIGNAL(returnPressed()), this->ui->pushButton, SLOT(click()));
}

GameInterface::~GameInterface()
{
    std::cout << "~gameinterface" << std::endl;
    if(gl->mode == MM_MODE) delete n;
    delete gl;
    delete ui;
}

/*
 * 开始网络游戏
 * 主要过程 发送匹配请求
 */
void GameInterface::startNetGame(){
    QString s;
    if(this->type == 1){
        //创建房间
        s = QString::number(CREATE_ROOM) + "\n" + me.getNickName() + "\n";
        this->ui->msg_list->clear();

        this->n->send(s);
    }else if(this->type == 2){
        s = QString::number(JOIN_ROOM) + "\n" + this->room_id + "\n" +me.getNickName() + "\n";
        this->ui->msg_list->clear();
//        this->addMsg("等待对方进入房间......");
        this->n->send(s);
    }else{
        s = QString::number(CONNECT) + "\n" + me.getNickName() + "\n";
        this->ui->msg_list->clear();
        this->addMsg("正在匹配对手......");
        this->n->send(s);
    }

//    if(!this->n->socket.waitForReadyRead()){
//        this->addMsg("人数较少，匹配失败！");
//        //TODO
//        //要么关掉 要么重开
//    }
}


void GameInterface::startGame(){
    this->ui->nickname_player->setText(me.getNickName());
    this->ui->msg_player->hide();
    this->ui->msg_competitor->hide();
    if(gl->mode == CM_MODE){
        this->status = STARTED;
        //人机模式 默认 玩家执白子
        this->mycolor = WHITE_PIECE;
        this->ui->msg_player->hide();
        this->ui->msg_competitor->hide();
        this->ui->nickname_competitor->setText("无敌大魔王");
        gl->startGame();
        gl->ai.gt = GameTree(9, 2, gl->ai.board);
        gl->ai.gt.game();
        auto pos = gl->ai.gt.get_next_pos();
        Chess ch(100 + pos.second * 30, 100 + pos.first * 30, BLACK_PIECE);
        this->gl->nextStep(ch);
        this->addMsg("游戏开始！你执白子！");
        this->addMsg("轮到你了");
    }
    else{
        //联机模式 在这里进行初始连接服务器
        n = new Net("127.0.0.1", 9999);
        connect(&n->socket, &QTcpSocket::connected, this, &GameInterface::connected);
        connect(&n->socket, &QTcpSocket::disconnected, this, &GameInterface::disconnected);
//        typedef void (QAbstractSocket::*QAbstractSocketErrorSignal)(QAbstractSocket::SocketError);
//        connect(&n->socket, static_cast<QAbstractSocketErrorSignal>(&QTcpSocket::error), this, &GameInterface::error);
//        connect(&n->socket, &QIODevice::readyRead, this, &GameInterface::readReady);
        connect(&n->socket, &QTcpSocket::readyRead, this, &GameInterface::readReady);
        if(!n->connect()){
            this->addMsg("连接服务器失败！");
            QMessageBox mess(QMessageBox::Information, tr("消息"), tr("连接服务器失败！"));
            QPushButton *quit = (mess.addButton(tr("退出"), QMessageBox::AcceptRole));
            mess.exec();
            if(mess.clickedButton() == quit){
                this->close();
            }
        }

        this->startNetGame();
    }

}

void GameInterface::setPlayer(Player p){
    this->me = p;
}


void GameInterface::setMode(int m){
    this->gl->setMode(m);
}


/*
 * 通过重写paintEvent函数 实现绘图
 */
void GameInterface::paintEvent(QPaintEvent *event){
    Q_UNUSED(event);
    QPainter painter(this);
    // 反走样
    painter.setRenderHint(QPainter::Antialiasing, true);
    //绘制整个棋盘
    // 设置画笔颜色、宽度
    painter.setPen(QPen(QColor(0, 0, 0), 2));
    // 设置画刷颜色
    painter.setBrush(QColor(197, 132, 68));
    painter.drawRect(90, 90, 440, 440);

    //绘制其中的线
    // 设置画笔颜色
    painter.setPen(QPen(QColor(0, 0, 0), 1));
    // 绘制棋盘
    for(int i = 0; i < 15; i++){
        painter.drawLine(QPointF(100 + i * 30, height() - 100), QPointF(100 + i * 30, 100));
    }
    for(int i = 0; i < 15; i++){
        painter.drawLine(QPointF(width() - 300, 100 + i * 30), QPointF(100, 100 + i * 30));
    }
    //绘制天元等
    // 设置画笔颜色、宽度
    painter.setPen(QPen(QColor(0, 0, 0), 1));
    // 设置画刷颜色
    painter.setBrush(QColor(0, 0, 0));
    // 绘制
    painter.drawEllipse(QPointF(190, 190), 3, 3);
    painter.drawEllipse(QPointF(310, 310), 3, 3);
    painter.drawEllipse(QPointF(430, 430), 3, 3);
    painter.drawEllipse(QPointF(430, 190), 3, 3);
    painter.drawEllipse(QPointF(190, 430), 3, 3);

    //绘制落子情况
    // 反走样
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 绘制棋子
    for(int i = 0; i < this->gl->chessOrder.size(); i++){
        if(this->gl->chessOrder[i].colour == BLACK_PIECE){
            painter.setPen(QPen(QColor(0, 0, 0), 1));
            painter.setBrush(QColor(0, 0, 0));
        }else{
            painter.setPen(QPen(QColor(255, 255, 255), 1));
            painter.setBrush(QColor(255, 255, 255));
        }

        painter.drawEllipse(QPointF(this->gl->chessOrder[i].x, this->gl->chessOrder[i].y), 10, 10);
    }
}

/*
 * 通过重写mousePressEventH函数 来实现鼠标点击 下棋
 */
void GameInterface::mousePressEvent(QMouseEvent *e){
    if(this->status != STARTED) return;
    if(!(e->x() > 90 && e->x() < 530 && e->y() > 90 && e->y() < 530)){
        //点击在棋盘外 不做反应
        return;
    }
    else{
        //TODO
        //调用gamelogic中的nextStep 返回true 说明 点击有效
        //                            false 说明点击无效 不做反应
        Chess c(e->x(), e->y(), this->mycolor);
        if(this->gl->nextStep(c)){
            repaint();
            //我落子后判断有没有胜利
            gl->checkWin();
            if(gl->isWin == BLACK_WIN){
                this->addMsg("黑方 胜利");
                if(gl->mode == MM_MODE){
                    //如果是联机对战
                    //发送落子消息
                    qDebug() << c.x << ' ' << c.y << '\n';
                    QString s = QString::number(SETPIECE) + "\n" + QString::number(c.x) + "\n" + QString::number(c.y) + "\n" + QString::number(c.colour) + "\n";
//                    QString ss = QString::number(SET)
                    qDebug() << s;
                    this->n->send(s);
                    //发送结束消息
                    s = QString::number(GAMEOVER) + "\n" + QString::number(this->mycolor) + "\n";
                    this->n->send(s);
                    this->status = WAIT_START;
                }
                //弹出对话框
                QMessageBox mess(QMessageBox::Information, tr("消息"), tr("你赢了！黑方 胜利！"));
                QPushButton *restart = (mess.addButton(tr("重开"), QMessageBox::YesRole));
                QPushButton *quit = (mess.addButton(tr("退出"), QMessageBox::AcceptRole));
                mess.exec();
                if(mess.clickedButton() == restart){
                    if(gl->mode == MM_MODE){
                        //联机模式 重新匹配
                        this->startNetGame();
                    }else{
                        this->startGame();
                        repaint();
                    }
                }else{
                    this->close();
                }
            }else if(gl->isWin == WHITE_WIN){
                this->addMsg("白方 胜利");
                if(gl->mode == MM_MODE){
                    //如果是联机对战
                    //发送落子消息
                    qDebug() << c.x << ' ' << c.y << '\n';
                    QString s = QString::number(SETPIECE) + "\n" + QString::number(c.x) + "\n" + QString::number(c.y) + "\n" + QString::number(c.colour) + "\n";
                    qDebug() << s;
                    this->n->send(s);
                    //发送结束消息
                    s = QString::number(GAMEOVER) + "\n" + QString::number(this->mycolor) + "\n";
                    this->n->send(s);
                    this->status = WAIT_START;
                }
                QMessageBox mess(QMessageBox::Information, tr("消息"), tr("你赢了！白方 胜利！"));
                QPushButton *restart = (mess.addButton(tr("重开"), QMessageBox::YesRole));
                QPushButton *quit = (mess.addButton(tr("退出"), QMessageBox::AcceptRole));
                mess.exec();
                if(mess.clickedButton() == restart){
                    if(gl->mode == MM_MODE){
                        this->startNetGame();
                    }else{
                        this->startGame();
                        repaint();
                    }
                }else{
                    this->close();
                }
            }else{
                //没有胜利
                this->addMsg("等待对方落子！");
                if(gl->mode == CM_MODE){
                    //如果是人机对战 在这里进行 人机落子
                    //将颜色变为人机一方
                    if(this->mycolor == BLACK_PIECE) this->mycolor = WHITE_PIECE;
                    else this->mycolor = BLACK_PIECE;
                    //人机落子
                    gl->ai.gt = GameTree(9, 2, gl->ai.board);
                    gl->ai.gt.game();
                    //人工智能计算 落子
                    auto pos = gl->ai.gt.get_next_pos();
                    Chess ch(100 + pos.second * 30, 100 + pos.first * 30, this->mycolor);
                    this->gl->nextStep(ch);
                    repaint();
                    //将颜色变回来
                    if(this->mycolor == BLACK_PIECE) this->mycolor = WHITE_PIECE;
                    else this->mycolor = BLACK_PIECE;

                    //落子后判断有没有胜利  因为是人机 只能是黑子  只有可能黑子获胜
                    gl->checkWin();
                    if(gl->isWin == BLACK_WIN){
                        this->addMsg("黑方 胜利");
                        QMessageBox mess(QMessageBox::Information, tr("消息"), tr("你输了！黑方 胜利！"));
                        QPushButton *restart = (mess.addButton(tr("重开"), QMessageBox::YesRole));
                        QPushButton *quit = (mess.addButton(tr("退出"), QMessageBox::AcceptRole));
                        mess.exec();
                        if(mess.clickedButton() == restart){
                            this->startGame();
                            repaint();
                        }else{
                            this->close();
                        }
                    }
                    else{
                        this->addMsg("轮到你了！");
                    }

                }else{
                    //如果是联机对战 在这里把自己的落子发送给服务器
                    qDebug() << c.x << ' ' << c.y << '\n';
                    QString s = QString::number(SETPIECE) + "\n" + QString::number(c.x) + "\n" + QString::number(c.y) + "\n" + QString::number(c.colour) + "\n";
                    qDebug() << s;
                    this->n->send(s);
                }
            }
        }
    }
}

/*
 * 消息框接口
 */
void GameInterface::addMsg(QString s){
    if(s == "") return;
    this->ui->msg_list->addItem(s);
    this->ui->msg_list->scrollToBottom();
}

/*
 * 悔棋按钮槽函数
 */
void GameInterface::pressButtonBack(){
    if(gl->mode == CM_MODE){
        gl->drawGame(this->mycolor);
        repaint();
    }else{
        if(this->status == WAIT_START){
            //游戏没有正是开始
            this->addMsg("游戏还未开始！");
            return;
        }
        if(this->status == WAIT_TIE){
            this->addMsg("等待对方响应！");
            return;
        }
        if(this->status == WAIT_UNDO){
            this->addMsg("等待对方响应！");
            return;
        }
        QString s = QString::number(UNDO) + "\n";
        this->n->send(s);
        this->status = WAIT_UNDO;
    }
}

/*
 * 求和按钮槽函数
 */
void GameInterface::pressButtonDraw(){
    if(gl->mode == CM_MODE){
        this->addMsg("平局！");
        QMessageBox mess(QMessageBox::Information, tr("消息"), tr("对方同意求和！平局！"));
        QPushButton *restart = (mess.addButton(tr("重开"), QMessageBox::YesRole));
        QPushButton *quit = (mess.addButton(tr("退出"), QMessageBox::AcceptRole));
        mess.exec();
        if(mess.clickedButton() == restart){
            this->startGame();
            repaint();
        }else{
            this->close();
        }
    }else{
        if(this->status == WAIT_START){
            //游戏没有正是开始
            this->addMsg("游戏还未开始！");
            return;
        }
        if(this->status == WAIT_TIE){
            this->addMsg("等待对方响应！");
            return;
        }
        if(this->status == WAIT_UNDO){
            this->addMsg("等待对方响应！");
            return;
        }
        QString s = QString::number(TIE) + "\n";
        this->n->send(s);
        this->status = WAIT_TIE;
    }
}

/*
 * 认输按钮槽函数
 */
void GameInterface::pressButtonSurrend(){
    if(gl->mode == CM_MODE){
        this->addMsg("黑方 胜利!");
        QMessageBox mess(QMessageBox::Information, tr("消息"), tr("你输了！黑方 胜利！"));
        QPushButton *restart = (mess.addButton(tr("重开"), QMessageBox::YesRole));
        QPushButton *quit = (mess.addButton(tr("退出"), QMessageBox::AcceptRole));
        mess.exec();
        if(mess.clickedButton() == restart){
            this->startGame();
            repaint();
        }else{
            this->close();
        }
    }else{
        if(this->status == WAIT_START){
            //游戏没有正是开始
            this->addMsg("游戏还未开始！");
            return;
        }
        if(this->status == WAIT_TIE){
            this->addMsg("等待对方响应！");
            return;
        }
        if(this->status == WAIT_UNDO){
            this->addMsg("等待对方响应！");
            return;
        }

        QString s = QString::number(SURRENDER) + "\n";
        this->n->send(s);
        this->status = WAIT_START;

        this->addMsg("对方 胜利!");
        QMessageBox mess(QMessageBox::Information, tr("消息"), tr("你输了！对方 胜利！"));
        QPushButton *restart = (mess.addButton(tr("重开"), QMessageBox::YesRole));
        QPushButton *quit = (mess.addButton(tr("退出"), QMessageBox::AcceptRole));
        mess.exec();
        if(mess.clickedButton() == restart){
            this->startNetGame();
            repaint();
        }else{
            this->close();
        }
    }
}

/*
 * 发送按钮槽函数
 */
void GameInterface::enter(){
    QString text = this->ui->lineEdit->text();
    if(text.length() > 50){
        this->addMsg("消息过长！");
        return;
    }
    if(text == "") return;
    this->ui->lineEdit->clear();

    if(gl->mode == MM_MODE){
        if(this->status == WAIT_START){
            //游戏没有正是开始
            this->addMsg("游戏还未开始！");
            return;
        }
        if(this->status == WAIT_TIE){
            this->addMsg("等待对方响应！");
            return;
        }
        if(this->status == WAIT_UNDO){
            this->addMsg("等待对方响应！");
            return;
        }
        QString s = QString::number(MSG) + '\n' + text + '\n';
        n->send(s);
        this->addMsg("我：" + text);
    }else{
        this->addMsg("我：" + text);
    }
}


/*
 * 消息 接收 处理函数
 */
void GameInterface::readReady(){
    QString s = n->read();
    qDebug() << s;
    QStringList list = s.split('\n');
    int code = list[0].toInt();
    switch(code){
        case CONNECT_SUCCESS:{
            if(this->type == 2){
                this->addMsg("加入房间成功！");
            }else if(this->type == 1){
                this->addMsg("对方已进入房间！");
            }else{
                this->addMsg("匹配成功！");
            }

            //匹配成功 开始游戏
            //设置对手昵称
            this->ui->nickname_competitor->setText(list[1]);
            if(list[2].toInt() == BLACK_PIECE){
                this->addMsg("你执黑子，游戏开始！黑子先！");
            }else{
               this->addMsg("你执白子，游戏开始！黑子先！");
            }
            this->mycolor = list[2].toInt();
            this->ui->label_2->setText("房间：" + list[3]);
            //将棋盘状态 设置为开始
            this->status = STARTED;
            gl->startGame();
            repaint();
            break;
        }
        case CONNECT_FAIL:{
            //匹配失败
            break;
        }
        case SETPIECE:{
            //如果 游戏不是已近开始状态 忽略
            if(this->status != STARTED) break;
            //对方落子
            Chess c(list[1].toInt(), list[2].toInt(), list[3].toInt());
            if(this->gl->nextStep(c)){
                repaint();
                this->addMsg("轮到你了！");
            }else{
                cout << "error set piece" << endl;
            }
            break;
        }
        case UNDO:{
            //如果 游戏不是已近开始状态 忽略
            if(this->status != STARTED) break;
            //悔棋请求
            //弹出对话框
            QMessageBox mess(QMessageBox::Information, tr("消息"), tr("对方请求悔棋！"));
            QPushButton *yes = (mess.addButton(tr("同意"), QMessageBox::YesRole));
            QPushButton *no = (mess.addButton(tr("不同意"), QMessageBox::AcceptRole));
            mess.exec();
            if(mess.clickedButton() == yes){
                //同意对方悔棋
                if(mycolor == BLACK_PIECE) gl->drawGame(WHITE_PIECE);
                else gl->drawGame(BLACK_PIECE);
                this->addMsg("您同意对方悔棋，等待对方落子！");
                repaint();
                QString s = QString::number(UNDO_YES) + '\n';
                n->send(s);
            }else{
                QString s = QString::number(UNDO_NO) + '\n';
                n->send(s);
            }
            break;
        }
        case UNDO_YES:{
            //如果 游戏不是等待响应状态 忽略
            if(this->status != WAIT_UNDO) break;
            //对方同意悔棋
            this->addMsg("对方同意悔棋！请落子！");
            //将棋盘状态 设置为开始
            this->status = STARTED;
            gl->drawGame(mycolor);
            repaint();
            break;
        }
        case UNDO_NO:{
            //如果 游戏不是等待响应状态 忽略
            if(this->status != WAIT_UNDO) break;
            //对方拒绝悔棋
            this->addMsg("对方拒绝悔棋！");
            //将棋盘状态 设置为开始
            this->status = STARTED;
            break;
        }
        case GAMEOVER:{
            //如果 游戏不是已近开始状态 忽略
            if(this->status != STARTED) break;
            //将棋盘状态 设置为等待开始
            this->status = WAIT_START;
            //游戏结束 对方胜利
            // code\ncolor\n
            if(list[1].toInt() == BLACK_PIECE){
                this->addMsg("黑方 胜利");
                QMessageBox mess(QMessageBox::Information, tr("消息"), tr("你输了！黑方 胜利！"));
                QPushButton *restart = (mess.addButton(tr("重开"), QMessageBox::YesRole));
                QPushButton *quit = (mess.addButton(tr("退出"), QMessageBox::AcceptRole));
                mess.exec();
                if(mess.clickedButton() == restart){
                    this->startNetGame();
                    repaint();
                }else{
                    this->close();
                }
            }else{
                this->addMsg("白方 胜利");
                QMessageBox mess(QMessageBox::Information, tr("消息"), tr("你输了！白方 胜利！"));
                QPushButton *restart = (mess.addButton(tr("重开"), QMessageBox::YesRole));
                QPushButton *quit = (mess.addButton(tr("退出"), QMessageBox::AcceptRole));
                mess.exec();
                if(mess.clickedButton() == restart){
                    this->startNetGame();
                    repaint();
                }else{
                    this->close();
                }
            }
            break;
        }
        case DISCONNECT:{
            //对方断开连接
            this->addMsg("对方断开连接！");
            QMessageBox mess(QMessageBox::Information, tr("消息"), tr("对方断开连接！"));
//            QPushButton *restart = (mess.addButton(tr("重新匹配"), QMessageBox::YesRole));
            QPushButton *quit = (mess.addButton(tr("退出"), QMessageBox::AcceptRole));
            mess.exec();
            if(mess.clickedButton() == quit ){
                this->close();
            }
            break;
        }
        case SURRENDER:{
            //如果 游戏不是已近开始状态 忽略
            if(this->status != STARTED) break;
            //将棋盘状态 设置为开始
            this->status = WAIT_START;
            //对方投降 游戏结束
            this->addMsg("对方投降！");
            QMessageBox mess(QMessageBox::Information, tr("消息"), tr("对方投降！你赢了！"));
            QPushButton *restart = (mess.addButton(tr("重新匹配"), QMessageBox::YesRole));
            QPushButton *quit = (mess.addButton(tr("退出"), QMessageBox::AcceptRole));
            mess.exec();
            if(mess.clickedButton() == restart){
                this->startNetGame();
                repaint();
            }else{
                this->close();
            }
            break;
        }
        case TIE:{

            //如果 游戏不是等待响应状态 忽略
            if(this->status != STARTED) break;
            //求和请求
            //弹出对话框
            QMessageBox mess(QMessageBox::Information, tr("消息"), tr("对方请求和棋！"));
            QPushButton *yes = (mess.addButton(tr("同意"), QMessageBox::YesRole));
            QPushButton *no = (mess.addButton(tr("不同意"), QMessageBox::AcceptRole));
            mess.exec();
            if(mess.clickedButton() == yes){
                //同意对方和棋
                QString s = QString::number(TIE_YES) + '\n';
                n->send(s);
                QMessageBox messs(QMessageBox::Information, tr("消息"), tr("平局！"));
                QPushButton *restart = (messs.addButton(tr("重新匹配"), QMessageBox::YesRole));
                QPushButton *quit = (messs.addButton(tr("退出"), QMessageBox::AcceptRole));
                messs.exec();
                if(messs.clickedButton() == restart){
                    this->startNetGame();
                    repaint();
                }else{
                    this->close();
                }
            }else{
                QString s = QString::number(TIE_NO) + '\n';
                n->send(s);
            }
            break;
        }
        case TIE_YES:{
            //如果 游戏不是等待响应状态 忽略
            if(this->status != WAIT_TIE) break;
            //将棋盘状态 设置为开始
            this->status = STARTED;
            //同意求和 双方平局
            QMessageBox messs(QMessageBox::Information, tr("消息"), tr("平局！"));
            QPushButton *restart = (messs.addButton(tr("重新匹配"), QMessageBox::YesRole));
            QPushButton *quit = (messs.addButton(tr("退出"), QMessageBox::AcceptRole));
            messs.exec();
            if(messs.clickedButton() == restart){
                this->startNetGame();
                repaint();
            }else{
                this->close();
            }
            break;
        }
        case TIE_NO:{
            //如果 游戏不是等待响应状态 忽略
            if(this->status != WAIT_TIE) break;
            //拒绝求和
            this->addMsg("对方拒绝求和！");
            //将棋盘状态 设置为开始
            this->status = STARTED;
            break;
        }
        case MSG:{
            //如果 游戏不是开始状态 忽略
            if(this->status != STARTED) break;
            //对方发送消息
            this->addMsg(this->ui->nickname_competitor->text() + "：" + list[1]);
            break;
        }
        case JOIN_ROOM_FAIL:{
            this->addMsg("没有该房间号！");
            QMessageBox mess(QMessageBox::Information, tr("消息"), tr("没有该房间号"));
            QPushButton *quit = (mess.addButton(tr("退出"), QMessageBox::AcceptRole));
            mess.exec();
            if(mess.clickedButton() == quit){
                this->close();
            }
            break;
        }
        case CREATE_ROOM_SUCCESS:{
            this->addMsg("创建房间成功！房间号：" + list[1]);
            this->addMsg("等待对方进入房间......");
            this->ui->label_2->setText("房间：" + list[1]);
        }
    }

    this->addMsg(n->read());
}
void GameInterface::connected(){
    this->addMsg("连接服务器成功！");
}
void GameInterface::disconnected(){
    this->addMsg("服务器断开！");
}
void GameInterface::error(QAbstractSocket::SocketError){

}
