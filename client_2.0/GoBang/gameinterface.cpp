#include "gameinterface.h"
#include "ui_gameinterface.h"

GameInterface::GameInterface(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::GameInterface)
{
    ui->setupUi(this);
    //设置窗口属性调用 close()时 调用析构函数
    //setAttribute(Qt::WA_DeleteOnClose);
    //设置窗口为应用程序 打开此窗口时 阻塞其他窗口
    setWindowModality(Qt::ApplicationModal);
    //信号槽连接
    connect(this->ui->pushButton, SIGNAL(clicked()), this, SLOT(enter()));
    connect(this->ui->pushButton_back, SIGNAL(clicked()), this, SLOT(pressButtonBack()));
    connect(this->ui->pushButton_draw, SIGNAL(clicked()), this, SLOT(pressButtonDraw()));
    connect(this->ui->pushButton_surrend, SIGNAL(clicked()), this, SLOT(pressButtonSurrend()));
    connect(this->ui->lineEdit_msg, SIGNAL(returnPressed()), this->ui->pushButton, SLOT(click()));
}

GameInterface::~GameInterface()
{
    std::cout << "~gameinterface" << std::endl;
    delete ui;
}

void GameInterface::setN(Net *net){
    this->n = net;
}

/**
 * @brief 收到匹配成功 调用该接口
 * @param rid 房间号
 */
void GameInterface::matchSuccess(std::string &rid){
    this->room_id = QString::fromStdString(rid);

    this->status = WAIT_START;

    this->ui->label_rid->setText("房间号：" + this->room_id);
    this->ui->msg_list->clear();
    this->ui->lable_me_turn->setHidden(true);
    this->ui->lable_competitor_turn->setHidden(true);

    addMsg("系统","匹配成功！");
    memset(this->board, 0, sizeof(this->board));
    this->show();
}
void GameInterface::createRoomSuccess(std::string &rid){
    this->room_id = QString::fromStdString(rid);

    this->status = WAIT_START;

    this->ui->label_rid->setText("房间号：" + this->room_id);
    this->ui->msg_list->clear();
    this->ui->lable_me_turn->setHidden(true);
    this->ui->lable_competitor_turn->setHidden(true);

    this->ui->nickname_player->setText(QString::fromStdString(me.name()));

    addMsg("系统","创建成功！等待玩家进入！");
    memset(this->board, 0, sizeof(this->board));
    this->show();
}
void GameInterface::joinRoomSuccess(std::string &cname, std::string &rid){
    this->room_id = QString::fromStdString(rid);

    this->status = WAIT_START;

    this->ui->label_rid->setText("房间号：" + this->room_id);
    this->ui->msg_list->clear();
    this->ui->lable_me_turn->setHidden(true);
    this->ui->lable_competitor_turn->setHidden(true);

    this->ui->nickname_player->setText(QString::fromStdString(me.name()));
    this->ui->nickname_competitor->setText(QString::fromStdString(cname));

    addMsg("系统","匹配成功！");
    memset(this->board, 0, sizeof(this->board));
    this->show();
}

void GameInterface::someoneJoinRoom(std::string name, bool isReconnect){
    QString qname = QString::fromStdString(name);

    if(!isReconnect){
        this->status = WAIT_START;
        addMsg("系统",qname + " 加入房间！");
    }
    else{
        this->status = STARTED; //对方重连 将状态变为开始游戏
        addMsg("系统",qname + " 重连进入房间！");
    }

}

/**
 * @brief 收到开始游戏调用该接口
 * @param m 我的信息
 * @param mclr  我的棋子颜色
 * @param c 对手信息
 * @param cclr 对手棋子颜色
 */
void GameInterface::gameStart(GoBang::Player &m, GoBang::PieceColor &mclr, GoBang::Player &c, GoBang::PieceColor &cclr){
    this->me = m;
    this->myColor = mclr;
    this->competitor = c;
    this->competitorColor = cclr;

    this->status = STARTED;
    this->nextColor = GoBang::NO_PIECE;
    //绘制画面
    this->ui->nickname_player->setText(QString::fromStdString(me.name()));
    this->ui->nickname_competitor->setText(QString::fromStdString(competitor.name()));

    this->ui->lable_me_turn->setHidden(true);
    this->ui->lable_competitor_turn->setHidden(true);

    addMsg("系统", "游戏开始！");
    if(mclr == GoBang::BLACK) addMsg("系统", "你执黑子！黑子先手！");
    else addMsg("系统", "你执白子！白子后手！");
    memset(this->board, 0, sizeof(this->board));
    update();
}

/**
 * @brief setPiece 接口
 * @param nextclr 下一个棋子的颜色
 * @param b 棋盘信息
 */
void GameInterface::setPiece(GoBang::PieceColor &nextclr, GoBang::Border &b){
    this->nextColor = nextclr;
    int i = 0;
    int64_t tmp = 0x00000000000000000000000000000003;
    int64_t r2 = b.row_2();
    int64_t r4 = b.row_4();
    int64_t r6 = b.row_6();
    int64_t r8 = b.row_8();
    int64_t r10 = b.row_10();
    int64_t r12 = b.row_12();
    int64_t r14 = b.row_14();
    int64_t r16 = b.row_16();

    for(i = 0; i < 2; i++){
        for(int j = 0; j < 15; j++){
            if((r2 & tmp) == GoBang::NO_PIECE){
                board[i][j] = GoBang::NO_PIECE;
            }
            else if((r2 & tmp) == GoBang::BLACK){
                board[i][j] = GoBang::BLACK;
            }
            else if((r2 & tmp) == GoBang::WHITE){
                board[i][j] = GoBang::WHITE;
            }
            r2 >>= 2;
        }
    }
    for(i = 2; i < 4; i++){
        for(int j = 0; j < 15; j++){
            if((r4 & tmp) == GoBang::NO_PIECE){
                board[i][j] = GoBang::NO_PIECE;
            }
            else if((r4 & tmp) == GoBang::BLACK){
                board[i][j] = GoBang::BLACK;
            }
            else if((r4 & tmp) == GoBang::WHITE){
                board[i][j] = GoBang::WHITE;
            }
            r4 >>= 2;
        }
    }
    for(i = 4; i < 6; i++){
        for(int j = 0; j < 15; j++){
            if((r6 & tmp) == GoBang::NO_PIECE){
                board[i][j] = GoBang::NO_PIECE;
            }
            else if((r6 & tmp) == GoBang::BLACK){
                board[i][j] = GoBang::BLACK;
            }
            else if((r6 & tmp) == GoBang::WHITE){
                board[i][j] = GoBang::WHITE;
            }
            r6 >>= 2;
        }
    }
    for(i = 6; i < 8; i++){
        for(int j = 0; j < 15; j++){
            if((r8 & tmp) == GoBang::NO_PIECE){
                board[i][j] = GoBang::NO_PIECE;
            }
            else if((r8 & tmp) == GoBang::BLACK){
                board[i][j] = GoBang::BLACK;
            }
            else if((r8 & tmp) == GoBang::WHITE){
                board[i][j] = GoBang::WHITE;
            }
            r8 >>= 2;
        }
    }
    for(i = 8; i < 10; i++){
        for(int j = 0; j < 15; j++){
            if((r10 & tmp) == GoBang::NO_PIECE){
                board[i][j] = GoBang::NO_PIECE;
            }
            else if((r10 & tmp) == GoBang::BLACK){
                board[i][j] = GoBang::BLACK;
            }
            else if((r10 & tmp) == GoBang::WHITE){
                board[i][j] = GoBang::WHITE;
            }
            r10 >>= 2;
        }
    }
    for(i = 10; i < 12; i++){
        for(int j = 0; j < 15; j++){
            if((r12 & tmp) == GoBang::NO_PIECE){
                board[i][j] = GoBang::NO_PIECE;
            }
            else if((r12 & tmp) == GoBang::BLACK){
                board[i][j] = GoBang::BLACK;
            }
            else if((r12 & tmp) == GoBang::WHITE){
                board[i][j] = GoBang::WHITE;
            }
            r12 >>= 2;
        }
    }
    for(i = 12; i < 14; i++){
        for(int j = 0; j < 15; j++){
            if((r14 & tmp) == GoBang::NO_PIECE){
                board[i][j] = GoBang::NO_PIECE;
            }
            else if((r14 & tmp) == GoBang::BLACK){
                board[i][j] = GoBang::BLACK;
            }
            else if((r14 & tmp) == GoBang::WHITE){
                board[i][j] = GoBang::WHITE;
            }
            r14 >>= 2;
        }
    }
    for(i = 14; i < 15; i++){
        for(int j = 0; j < 15; j++){
            if((r16 & tmp) == GoBang::NO_PIECE){
                board[i][j] = GoBang::NO_PIECE;
            }
            else if((r16 & tmp) == GoBang::BLACK){
                board[i][j] = GoBang::BLACK;
            }
            else if((r16 & tmp) == GoBang::WHITE){
                board[i][j] = GoBang::WHITE;
            }
            r16 >>= 2;
        }
    }

    update();
    if(nextColor == myColor){
        this->ui->lable_me_turn->setHidden(false);
        this->ui->lable_competitor_turn->setHidden(true);
    }
    else if(nextColor == competitorColor){
        this->ui->lable_me_turn->setHidden(true);
        this->ui->lable_competitor_turn->setHidden(false);
    }
    else{
        this->ui->lable_me_turn->setHidden(true);
        this->ui->lable_competitor_turn->setHidden(true);
    }
}

/*
 * 消息框接口
 */
void GameInterface::addMsg(QString name, QString msg){
    if(msg == "") return;
    this->ui->msg_list->addItem(name + "：" + msg);
    this->ui->msg_list->scrollToBottom();
}

void GameInterface::undoRequest(){
    char buf[1024];
    int length = 0;
    int *ptr = &length;
    QMessageBox mess(QMessageBox::Information, tr("消息"), tr("对方请求悔棋！"));
    QPushButton *yes = (mess.addButton(tr("同意"), QMessageBox::YesRole));
    QPushButton *no = (mess.addButton(tr("拒绝"), QMessageBox::NoRole));
    mess.exec();
    if(mess.clickedButton() == yes){
        GoBang::GoBangRequest req;
        req.set_type(GoBang::UNDO);
        GoBang::UndoResponse *undoresp = req.mutable_undoresp();
        undoresp->set_success(true);
        undoresp->set_rid(this->room_id.toStdString());
        undoresp->set_uid(me.uid());
        std::cout << "undoresp--" << undoresp->success() << '\n';
        //序列化
        std::string s = req.SerializeAsString();
        //封包 发送
        memset(buf, 0, sizeof(buf));
        length = s.length();
        memcpy(buf, ptr, sizeof(int));
        memcpy(buf + 4, s.c_str(), length);
        n->socket.write(buf, length + 4);
        std::cout << "undoresp--4--" << length << '\n';
    }
    else{
        GoBang::GoBangRequest req;
        req.set_type(GoBang::UNDO);
        GoBang::UndoResponse *undoresp = req.mutable_undoresp();
        undoresp->set_success(false);
        undoresp->set_rid(this->room_id.toStdString());
        undoresp->set_uid(me.uid());
        //序列化
        std::string s = req.SerializeAsString();
        //封包 发送
        memset(buf, 0, sizeof(buf));
        length = s.length();
        memcpy(buf, ptr, sizeof(int));
        memcpy(buf + 4, s.c_str(), length);
        n->socket.write(buf, length + 4);
    }
}

void GameInterface::tieRequest(){
    char buf[1024];
    int length = 0;
    int *ptr = &length;
    QMessageBox mess(QMessageBox::Information, tr("消息"), tr("对方请求求和！"));
    QPushButton *yes = (mess.addButton(tr("同意"), QMessageBox::YesRole));
    QPushButton *no = (mess.addButton(tr("拒绝"), QMessageBox::NoRole));
    mess.exec();
    if(mess.clickedButton() == yes){
        GoBang::GoBangRequest req;
        req.set_type(GoBang::TIE);
        GoBang::TieResponse *tieresp = req.mutable_tieresp();
        tieresp->set_success(true);
        tieresp->set_rid(this->room_id.toStdString());
        tieresp->set_uid(me.uid());
        //序列化
        std::string s = req.SerializeAsString();
        //封包 发送
        memset(buf, 0, sizeof(buf));
        length = s.length();
        memcpy(buf, ptr, sizeof(int));
        memcpy(buf + 4, s.c_str(), length);
        n->socket.write(buf, length + 4);
    }
    else{
        GoBang::GoBangRequest req;
        req.set_type(GoBang::TIE);
        GoBang::TieResponse *tieresp = req.mutable_tieresp();
        tieresp->set_success(false);
        tieresp->set_rid(this->room_id.toStdString());
        tieresp->set_uid(me.uid());
        //序列化
        std::string s = req.SerializeAsString();
        //封包 发送
        memset(buf, 0, sizeof(buf));
        length = s.length();
        memcpy(buf, ptr, sizeof(int));
        memcpy(buf + 4, s.c_str(), length);
        n->socket.write(buf, length + 4);
    }
}

void GameInterface::undoResponse(bool success){
    if(success) addMsg("系统", "对方同意悔棋");
    else addMsg("系统", "对方不同意悔棋");
    this->status = STARTED;
}

void GameInterface::tieResponse(bool success){

    if(success) addMsg("系统", "对方同意和棋");
    else addMsg("系统", "对方不同意和棋");
    this->status = STARTED;
}


void GameInterface::gameover(bool isWin, std::string &winUid,QString msg){
    if(msg != "") msg += '\n';
    if(isWin){
        if(winUid == "#") msg += "平局!";
        else msg += "你赢了！";
    }
    else msg += "你输了！";

    this->ui->lable_me_turn->setHidden(true);
    this->ui->lable_competitor_turn->setHidden(true);

    QMessageBox mess(QMessageBox::Information, tr("消息"), tr(msg.toStdString().c_str()));
    QPushButton *restart = (mess.addButton(tr("继续游戏"), QMessageBox::YesRole));
    QPushButton *quit = (mess.addButton(tr("退出房间"), QMessageBox::AcceptRole));
    mess.exec();
    this->status = WAIT_START;
    if(mess.clickedButton() == restart){
        addMsg("系统","等待对方选择是否继续游戏！");
        //发送继续游戏消息
        char buf[1024];
        int length = 0;
        int *ptr = &length;
        GoBang::GoBangRequest req;
        req.set_type(GoBang::CONTINUE_GAME);
        GoBang::ContinueGameRequest *coninuegamereq = req.mutable_continuegamereq();
        coninuegamereq->set_uid(me.uid());
        coninuegamereq->set_rid(this->room_id.toStdString());
        //序列化
        std::string s = req.SerializeAsString();
        //封包 发送
        memset(buf, 0, sizeof(buf));
        length = s.length();
        memcpy(buf, ptr, sizeof(int));
        memcpy(buf + 4, s.c_str(), length);
        n->socket.write(buf, length + 4);
        n->socket.readyRead();
    }
    else{
//        //发送退出房间消息
//        char buf[1024];
//        int length = 0;
//        int *ptr = &length;
//        GoBang::GoBangRequest req;
//        req.set_type(GoBang::QUIT_ROOM);
//        GoBang::QuitRoomRequest *quitroomreq = req.mutable_quitroomreq();
//        quitroomreq->set_uid(me.uid());
//        quitroomreq->set_rid(this->room_id.toStdString());
//        //序列化
//        std::string s = req.SerializeAsString();
//        //封包 发送
//        memset(buf, 0, sizeof(buf));
//        length = s.length();
//        memcpy(buf, ptr, sizeof(int));
//        memcpy(buf + 4, s.c_str(), length);
//        n->socket.write(buf, length + 4);
        this->close();
    }

}


void GameInterface::someoneQuitRoom(std::string &uid){
    if(uid == competitor.uid()){
        addMsg("系统", QString::fromStdString(competitor.name()) + " 退出房间");
        this->ui->nickname_competitor->setText("等待玩家进入！");
    }
    this->status = WAIT_START;
}

void GameInterface::someoneDisconnect(std::string &uid){
    if(uid == competitor.uid()){
        addMsg("系统", QString::fromStdString(competitor.name()) + " 断线等待对方重重连;2分钟后未重连将自动获胜！");
    }
    this->status = WAIT_CONNECT;
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
    for(int i = 0; i < BOARD_ROW; i++){
        for(int j = 0; j < BOARD_COL; j++){
            if(this->board[i][j] == GoBang::BLACK){
                painter.setPen(QPen(QColor(0, 0, 0), 1));
                painter.setBrush(QColor(0, 0, 0));
                painter.drawEllipse(QPointF(100 + j * 30, 100 + i * 30), 10, 10);
            }
            else if(this->board[i][j] == GoBang::WHITE){
                painter.setPen(QPen(QColor(255, 255, 255), 1));
                painter.setBrush(QColor(255, 255, 255));
                painter.drawEllipse(QPointF(100 + j * 30, 100 + i * 30), 10, 10);
            }


        }
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
        int row = int((e->y() - 100) / 30.0 + 0.5);
        int col = int((e->x() - 100) / 30.0 + 0.5);
        if(this->board[row][col] == GoBang::NO_PIECE){
            if(myColor == nextColor){
                //TODO  发送消息
                char buf[512];
                int length = 0;
                int *ptr = &length;
                GoBang::GoBangRequest req;
                req.set_type(GoBang::SET_PIECE);
                GoBang::SetPieceRequest *setpiecereq = req.mutable_setpiecereq();
                setpiecereq->set_rid(room_id.toStdString());
                GoBang::Piece *p = setpiecereq->mutable_p();
                p->set_x(row);
                p->set_y(col);
                p->set_color(myColor);

                //序列化
                std::string s = req.SerializeAsString();
                //封包 发送
                memset(buf, 0, sizeof(buf));
                length = s.length();
                memcpy(buf, ptr, sizeof(int));
                memcpy(buf + 4, s.c_str(), length);
                n->socket.write(buf, length + 4);
            }
            else{
                addMsg("系统", "不是你的回合！");
            }
        }
        else{
            addMsg("系统", "该位置不能落子！");
        }
    }
}


/*
 * 悔棋按钮槽函数
 */
void GameInterface::pressButtonBack(){
    if(this->status != STARTED){
        addMsg("系统", "现在不能操作！");
    }
    else{
        if(this->myColor != this->nextColor){
            addMsg("系统", "不是你的回合！");
        }
        else{
            char buf[1024];
            int length = 0;
            int *ptr = &length;
            GoBang::GoBangRequest req;
            req.set_type(GoBang::UNDO);
            GoBang::UndoRequest *undoreq = req.mutable_undoreq();
            undoreq->set_uid(me.uid());
            undoreq->set_rid(room_id.toStdString());
            //序列化
            std::string s = req.SerializeAsString();
            //封包 发送
            memset(buf, 0, sizeof(buf));
            length = s.length();
            memcpy(buf, ptr, sizeof(int));
            memcpy(buf + 4, s.c_str(), length);
            n->socket.write(buf, length + 4);

            this->status = WAIT_UNDO;
        }
    }
}

/*
 * 求和按钮槽函数
 */
void GameInterface::pressButtonDraw(){
    if(this->status != STARTED){
        addMsg("系统", "现在不能操作！");
    }
    else{
        if(this->myColor != this->nextColor){
            addMsg("系统", "不是你的回合！");
        }
        else{
            //发送求和请求
            char buf[1024];
            int length = 0;
            int *ptr = &length;
            GoBang::GoBangRequest req;
            req.set_type(GoBang::TIE);
            GoBang::TieRequest *tiereq = req.mutable_tiereq();
            tiereq->set_uid(me.uid());
            tiereq->set_rid(room_id.toStdString());
            //序列化
            std::string s = req.SerializeAsString();
            //封包 发送
            memset(buf, 0, sizeof(buf));
            length = s.length();
            memcpy(buf, ptr, sizeof(int));
            memcpy(buf + 4, s.c_str(), length);
            n->socket.write(buf, length + 4);

            this->status = WAIT_TIE;
        }
    }
}

/*
 * 认输按钮槽函数
 */
void GameInterface::pressButtonSurrend(){
    if(this->status != STARTED){
        addMsg("系统", "现在不能操作！");
    }
    else{
        if(this->myColor != this->nextColor){
            addMsg("系统", "不是你的回合！");
        }
        else{
            //发送认输消息
            char buf[1024];
            int length = 0;
            int *ptr = &length;
            GoBang::GoBangRequest req;
            req.set_type(GoBang::SURRENDER);
            GoBang::SurrenderRequest *surredreq = req.mutable_surrenderreq();
            surredreq->set_uid(me.uid());
            surredreq->set_rid(room_id.toStdString());
            //序列化
            std::string s = req.SerializeAsString();
            //封包 发送
            memset(buf, 0, sizeof(buf));
            length = s.length();
            memcpy(buf, ptr, sizeof(int));
            memcpy(buf + 4, s.c_str(), length);
            n->socket.write(buf, length + 4);

            this->status = WAIT_START;
        }
    }
}

/*
 * 发送按钮槽函数
 */
void GameInterface::enter(){
    if(this->status != STARTED){
        addMsg("系统", "现在不能操作！");
        return;
    }
    QString text = this->ui->lineEdit_msg->text();
    if(text.length() > 50){
        this->addMsg("系统","消息过长！");
        return;
    }
    if(text == "") return;
    this->ui->lineEdit_msg->clear();

    this->addMsg(QString::fromStdString(me.name()), text);
    //发送
    char buf[2048];
    int length = 0;
    int *ptr = &length;
    GoBang::GoBangRequest req;
    req.set_type(GoBang::MSG);
    GoBang::Messg *messg = req.mutable_messg();
    messg->set_uid(me.uid());
    messg->set_name(me.name());
    messg->set_rid(room_id.toStdString());
    messg->set_msg(text.toStdString());
    std::cout << "msg----" << messg->name() << ":" << messg->msg() << '\n';
    //序列化
    std::string s = req.SerializeAsString();
    std::cout << "msg----?" << s.length() << '\n';
    //封包 发送
    memset(buf, 0, sizeof(buf));
    length = s.length();
    memcpy(buf, ptr, sizeof(int));
    memcpy(buf + 4, s.c_str(), length);
    n->socket.write(buf, length + 4);
}


void GameInterface::closeEvent(QCloseEvent *event){
    //如果是断线 由menu中的disconnect函数 调用游戏界面的close 此时tcp连接已经断开 直接关闭
    if(!n->socket.isOpen()){
        event->accept();
        return;
    }

    if(this->status == STARTED || this->status == WAIT_TIE || this->status == WAIT_UNDO){
        //游戏正在进行中 退出游戏界面 跳出警告
        QMessageBox mess(QMessageBox::Information, tr("消息"), tr("游戏正在进行中，退出游戏将自动判输！"));
        QPushButton *yes = (mess.addButton(tr("退出"), QMessageBox::YesRole));
        QPushButton *no = (mess.addButton(tr("取消"), QMessageBox::NoRole));
        mess.exec();
        if(mess.clickedButton() == no){
            event->ignore();
            return;
        }
    }
    if(this->status == WAIT_CONNECT){
        //正在等待对方 退出游戏界面 跳出警告
        QMessageBox mess(QMessageBox::Information, tr("消息"), tr("正在等待对方重连，确认退出吗？"));
        QPushButton *yes = (mess.addButton(tr("退出"), QMessageBox::YesRole));
        QPushButton *no = (mess.addButton(tr("取消"), QMessageBox::NoRole));
        mess.exec();
        if(mess.clickedButton() == no){
            event->ignore();
            return;
        }
    }
    if(this->status == NO_GAME) {
        event->accept();
        return;
    }
    //发出退出房间信息
    char buf[1024];
    int length = 0;
    int *ptr = &length;
    GoBang::GoBangRequest req;
    req.set_type(GoBang::QUIT_ROOM);
    GoBang::QuitRoomRequest *quitroomreq = req.mutable_quitroomreq();
    quitroomreq->set_uid(me.uid());
    quitroomreq->set_rid(this->room_id.toStdString());
    //序列化
    std::string s = req.SerializeAsString();
    //封包 发送
    memset(buf, 0, sizeof(buf));
    length = s.length();
    memcpy(buf, ptr, sizeof(int));
    memcpy(buf + 4, s.c_str(), length);
    n->socket.write(buf, length + 4);
    this->status = NO_GAME;

    //再次请求 playermessage 和 ranklist
    //请求玩家数据
    //填充数据
    req.set_type(GoBang::PLAYER_MESSAGE);
    GoBang::PlayerMessageRequest *playerReq = req.mutable_playmessagereq();
    playerReq->set_uid(me.uid());

    //序列化
    s = req.SerializeAsString();
    //封包 发送
    memset(buf, 0, sizeof(buf));
    length = s.length();
    memcpy(buf, ptr, sizeof(int));
    memcpy(buf + 4, s.c_str(), length);
    n->socket.write(buf, length + 4);

    //请求ranklist数据
    req.set_type(GoBang::RANK_LIST);
    GoBang::RankListRequest *ranklistreq = req.mutable_ranklistreq();
    ranklistreq->set_cnt(20);
    ranklistreq->set_start(1);
    //序列化
    s = req.SerializeAsString();
    //封包 发送
    memset(buf, 0, sizeof(buf));
    length = s.length();
    memcpy(buf, ptr, sizeof(int));
    memcpy(buf + 4, s.c_str(), length);
    n->socket.write(buf, length + 4);

    event->accept();
    return;



//    this->parent->show();

}
