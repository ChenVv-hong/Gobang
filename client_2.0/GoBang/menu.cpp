#include "menu.h"
#include "ui_menu.h"

Menu::Menu(Net *net, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Menu),n(net),gameInterface(this)
{
    ui->setupUi(this);
    gameInterface.setN(n);
    this->setWindowModality(Qt::ApplicationModal);
    //信号和槽函数的连接
    connect(this->ui->pushButton_normal,SIGNAL(clicked()), this, SLOT(pressButtonNormal()));
    connect(this->ui->pushButton_rank,SIGNAL(clicked()), this, SLOT(pressButtonRank()));
    connect(this->ui->pushButton_create_room,SIGNAL(clicked()), this, SLOT(pressButtonCreateRoom()));
    connect(this->ui->pushButton_join_room,SIGNAL(clicked()), this, SLOT(pressButtonJoinRoom()));

}

Menu::~Menu()
{
    std::cout << "~Menu\n";
    delete ui;
}

void Menu::moveNetControl(){
    connect(&n->socket, &QTcpSocket::readyRead, this, &Menu::readReady);
    connect(&n->socket, &QTcpSocket::disconnected, this, &Menu::disconnected);
}

GoBang::Player& Menu::getMe(){
    return this->me;
}
/**
 * @brief M设置个人信息
 * @param p
 */
void Menu::setMe(GoBang::Player& p){
    this->me = p;
    this->ui->lable_uid->setText("uid：" + QString::fromStdString(p.uid()));
    this->ui->label_username->setText("用户名：" + QString::fromStdString(p.name()));
    this->ui->lable_points->setText("积分：" + QString::number(p.points()));
}

/**
 * @brief 设置ranklist信息
 * @param v
 */
void Menu::setRankList(QVector<GoBang::Player> &v){
    this->ui->RankList->clear();
    for(int i = 0; i < v.size(); i++){
        if(i == 0){
            PlayerItem *pitem = new PlayerItem();
            pitem->setData("名次", "uid", "用户名", "积分");
            QListWidgetItem *item = new QListWidgetItem();
            item->setSizeHint(QSize(420, 75));
            this->ui->RankList->addItem(item);
            this->ui->RankList->setItemWidget(item,pitem);
        }
        PlayerItem *pitem = new PlayerItem();
        pitem->setData(QString::number(i + 1), QString::fromStdString(v[i].uid()), QString::fromStdString(v[i].name()), QString::number(v[i].points()));
        QListWidgetItem *item = new QListWidgetItem();
        item->setSizeHint(QSize(470, 75));
        this->ui->RankList->addItem(item);
        this->ui->RankList->setItemWidget(item,pitem);
    }
}


/**
 * @brief 快速开始按钮 槽函数
 */
void Menu::pressButtonNormal(){
    std::cout << "normal match button\n";
    char buf[256];
    int length = 0;
    int *ptr = &length;
    GoBang::GoBangRequest req;
    req.set_type(GoBang::NORMAL_MATCH);
    GoBang::NormalMatchRequest *normalMatchReq = req.mutable_normalmatchreq();
    normalMatchReq->set_uid(me.uid());
    //序列化
    std::string s = req.SerializeAsString();
    //封包 发送
    memset(buf, 0, sizeof(buf));
    length = s.length();
    memcpy(buf, ptr, sizeof(int));
    memcpy(buf + 4, s.c_str(), length);
    n->socket.write(buf, length + 4);
    this->waitcreen.show();
}

/**
 * @brief 排位匹配槽函数
 */
void Menu::pressButtonRank(){
    std::cout << "rank match button\n";
    char buf[256];
    int length = 0;
    int *ptr = &length;
    GoBang::GoBangRequest req;
    req.set_type(GoBang::RANK_MATCH);
    GoBang::RankMatchRequest *rankMatchReq = req.mutable_rankmatchreq();
    rankMatchReq->set_uid(me.uid());
    //序列化
    std::string s = req.SerializeAsString();
    //封包 发送
    memset(buf, 0, sizeof(buf));
    length = s.length();
    memcpy(buf, ptr, sizeof(int));
    memcpy(buf + 4, s.c_str(), length);
    n->socket.write(buf, length + 4);
    this->waitcreen.show();
}

/**
 * @brief 创建房间按钮槽函数 发送消息
 */
void Menu::pressButtonCreateRoom(){
    std::cout << "create room button\n";
    char buf[256];
    int length = 0;
    int *ptr = &length;
    GoBang::GoBangRequest req;
    req.set_type(GoBang::CREATE_ROOM);
    GoBang::CreateRoomRequest *createRoomReq = req.mutable_createroomreq();
    createRoomReq->set_uid(me.uid());
    //序列化
    std::string s = req.SerializeAsString();
    //封包 发送
    memset(buf, 0, sizeof(buf));
    length = s.length();
    memcpy(buf, ptr, sizeof(int));
    memcpy(buf + 4, s.c_str(), length);
    n->socket.write(buf, length + 4);
}

/**
 * @brief 加入房间按钮槽函数 发送消息
 */
void Menu::pressButtonJoinRoom(){
    std::cout << "join room button\n";
    QRegExp regExpRid("^[0-9]{1,12}$");
    QString text = ui->lineEdit_rid->text();
    if(!regExpRid.exactMatch(text)){
        QMessageBox::StandardButton result=QMessageBox::critical(this, "错误", "只能是1~12位数字");
        return;
    }
    ui->lineEdit_rid->clear();
    std::string rid = text.toStdString();
    char buf[4096];
    int length = 0;
    int *ptr = &length;
    GoBang::GoBangRequest req;
    req.set_type(GoBang::JOIN_ROOM);
    GoBang::JoinRoomRequest *joinRoomReq = req.mutable_joinroomreq();
    joinRoomReq->set_uid(me.uid());
    joinRoomReq->set_rid(rid);
    //序列化
    std::string s = req.SerializeAsString();
    //封包 发送
    memset(buf, 0, sizeof(buf));
    length = s.length();
    memcpy(buf, ptr, sizeof(int));
    memcpy(buf + 4, s.c_str(), length);
    n->socket.write(buf, length + 4);
}
/**
 * @brief socket消息读取
 */
void Menu::readReady(){
    char len[4];
    char buf[4096];
    int length = 0;
    int *ptr = &length;
    std::vector<std::string> message;
    while(n->socket.bytesAvailable() > 4){
        memset(len, 0, sizeof(len));
        n->socket.read(len, 4);
        memcpy(ptr, len, sizeof(int));

        memset(buf, 0, sizeof(buf));
        std::cout << "____________" << 4 << "  " << length << '\n';
        n->socket.read(buf, std::min((int)sizeof(buf), length));
        message.emplace_back(std::string(buf, length));
    }

    //处理 响应数据
    GoBang::GoBangResponse goBangResponse;
    for(int i = 0; i < message.size(); i++){
        bool ret = goBangResponse.ParseFromString(message[i]);
        if(!ret){
            std::cout << "protobuf error";
            std::cout << "  " << message[i].length() << '\n';
            break;
        }
        switch(goBangResponse.type()){
        case(GoBang::PLAYER_MESSAGE):{
            //获取playermessage
            std::cout << "playermessage\n";
            GoBang::Player p = goBangResponse.playmessageresp().p();
            this->setMe(p);
            gameInterface.me = p;
            break;
        }
        case(GoBang::RANK_LIST):{
            //获取ranklist
            std::cout << "ranklist\n";
            GoBang::RankListResponse ranklistresp = goBangResponse.ranklistresp();
            bool success = ranklistresp.success();
            if(success){
                int size = ranklistresp.list_size();
                QVector<GoBang::Player> v;
                for(int j = 0; j < size; j++){
                    v.push_back(ranklistresp.list(j));
                }
                this->setRankList(v);
            }
            break;
        }
        case(GoBang::NORMAL_MATCH):{
            std::cout << "normal match\n";
            //快速开始 响应内容
            bool success = goBangResponse.normalmatchresp().success();
            if(success){
                this->waitcreen.close();
                std::string rid = goBangResponse.normalmatchresp().rid();
                this->gameInterface.matchSuccess(rid);
            }
            break;
        }
        case(GoBang::RANK_MATCH):{
            std::cout << "rank match\n";
            //排位匹配响应内容
            bool success = goBangResponse.rankmatchresp().success();
            if(success){
                this->waitcreen.close();
                std::string rid = goBangResponse.rankmatchresp().rid();
                this->gameInterface.matchSuccess(rid);
            }
            break;
        }
        case(GoBang::CREATE_ROOM):{
            std::cout << "create match\n";
            //创建房间响应内容
            bool success = goBangResponse.createroomresp().success();
            if(success){
                std::string rid = goBangResponse.createroomresp().rid();
                this->gameInterface.createRoomSuccess(rid);
            }
            break;
        }
        case(GoBang::JOIN_ROOM):{
            std::cout << "join room\n";
            //加入房间响应内容
            bool success = goBangResponse.joinroomresp().success();
            if(success){
                std::string rid = goBangResponse.joinroomresp().rid();
                std::string name= goBangResponse.joinroomresp().msg();
                this->gameInterface.joinRoomSuccess(name, rid);
            }
            else{
                QMessageBox::StandardButton result = QMessageBox::critical(this, "错误", QString::fromStdString(goBangResponse.joinroomresp().msg()));
            }
            break;
        }
        case(GoBang::SET_PIECE):{
            std::cout << "set piece\n";
            //同步棋盘内容
            bool success = goBangResponse.setpieceresp().success();
            if(success){
                GoBang::PieceColor next = goBangResponse.setpieceresp().next();
                GoBang::Border b =  goBangResponse.setpieceresp().b();
                this->gameInterface.setPiece(next, b);
            }
            break;
        }
        case(GoBang::UNDO):{
            std::cout << "undo\n";
            //悔棋响应内容
            if(goBangResponse.has_undoreq()){
                this->gameInterface.undoRequest();
            }
            if(goBangResponse.has_undoresp()){
                bool success = goBangResponse.undoresp().success();
                this->gameInterface.undoResponse(success);
            }
            break;
        }
        case(GoBang::TIE):{
            std::cout << "tie\n";
            //和棋响应内容
            if(goBangResponse.has_tieres()){
                this->gameInterface.tieRequest();
            }
            if(goBangResponse.has_tieresp()){
                bool success = goBangResponse.tieresp().success();
                this->gameInterface.tieResponse(success);
            }
            break;
        }
        case(GoBang::GAMEOVER):{
            std::cout << "game over\n";
            //游戏结束
            bool isWin = goBangResponse.gameover().iswin();
            std::string winUid = goBangResponse.gameover().winuid();
            std::string msg = goBangResponse.gameover().msg();
            this->gameInterface.gameover(isWin, winUid, QString::fromStdString(msg));
            break;
        }
        case(GoBang::GAMESTART):{
            std::cout << "game start\n";
            //游戏开始
            GoBang::PieceColor myColor = goBangResponse.gamestart().mycolor();
            GoBang::PieceColor competitorColor = goBangResponse.gamestart().competitorcolor();
            GoBang::Player competitor;
            competitor.set_uid(goBangResponse.gamestart().competitorid());
            competitor.set_name(goBangResponse.gamestart().competitorname());
            std::string rid = goBangResponse.gamestart().rid();
            this->gameInterface.gameStart(me,myColor,competitor, competitorColor);
            break;
        }
        case(GoBang::QUIT_ROOM):{
            std::cout << "quit room\n";
            //有人退出房间
            std::string uid = goBangResponse.quitroomresp().uid();
            std::string name = goBangResponse.quitroomresp().name();
            this->gameInterface.someoneQuitRoom(uid);
            break;
        }
        case(GoBang::SOMEONE_JOIN_ROOM):{
            std::cout << "some one join room\n";
            //有人加入房间
            bool reconnect = goBangResponse.someonejoinroomresp().reconnect();  //是否是重连用户
            std::string name = goBangResponse.someonejoinroomresp().name();
            this->gameInterface.someoneJoinRoom(name, reconnect);
            break;
        }
        case(GoBang::SOMEONE_DISCONNECT):{
            std::cout << "some one disconnect\n";
            std::string uid = goBangResponse.someonedisconnectresp().uid();
            std::string name = goBangResponse.someonedisconnectresp().name();
            this->gameInterface.someoneDisconnect(uid);
            break;
        }
        case(GoBang::MSG):{
            std::cout << "msg\n";
            std::string name = goBangResponse.messg().name();
            std::string msg = goBangResponse.messg().msg();
            std::cout << "???" << name << ':' << msg << '\n';
            gameInterface.addMsg(QString::fromStdString(name), QString::fromStdString(msg));
            break;
        }
        default:
            break;
        }
    }

}
/**
 * @brief socket连接断开 关闭游戏页面 关闭菜单界面
 */
void Menu::disconnected(){
    this->gameInterface.close();
    this->waitcreen.close();
    this->close();
}
