#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_test.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),n("127.0.0.1", 22725),menu(&n)
{
    ui->setupUi(this);
    this->t = LOGIN;
    //信号和槽函数连接
    connect(this->ui->pushButton_enter, SIGNAL(clicked()), this, SLOT(pressButton()));
    connect(this->ui->pushButton_switch, SIGNAL(clicked()), this, SLOT(switchButton()));
    connect(&n.socket, &QTcpSocket::readyRead, this, &MainWindow::readReady);
    connect(&n.socket, &QTcpSocket::connected, this, &MainWindow::connected);
    connect(&n.socket, &QTcpSocket::disconnected, this, &MainWindow::disconnected);

}

MainWindow::~MainWindow()
{
    std::cout << "~mainwinod\n";
    delete ui;
}

bool MainWindow::connectServer(){
    if(!n.connect()){
        n.socket.close();
        QMessageBox::StandardButton result=QMessageBox::critical(this, "错误", "无法连接到服务器！");
        return false;
    }
    return true;
}

void MainWindow::switchButton(){
    if(this->t == LOGIN){
        this->t = REGISTER;
        this->ui->label_subtitle->setText("注册");
        this->ui->pushButton_switch->setText("返回登陆");
        this->ui->pushButton_enter->setText("确定");
    }
    else if(this->t == REGISTER){
        this->t = LOGIN;
        this->ui->label_subtitle->setText("登陆");
        this->ui->pushButton_switch->setText("没有账号？注册一个");
        this->ui->pushButton_enter->setText("进入");
    }
}


/*
 * 进入按钮的槽函数
 */
void MainWindow::pressButton(){

    //    ^[A-Za-z0-9\u4e00-\u9fa5]{3,10}$  只包含中文，英文字母和数字，长度3～10之间
    //    ^[0-9a-zA-Z_]{4,12}$  只包含字母、数字和下划线，长度在4~12之间
    QRegExp regExpName("^[A-Za-z0-9\u4e00-\u9fa5]{3,8}$");
    QRegExp regExpPasswd("^[0-9a-zA-Z_]{4,12}$");
    QString name = ui->lineEdit_name->text();
    QString passwd = ui->lineEdit_passwd->text();
    QString info = "";
    char buf[4096];
    int length = 0;
    int *ptr = &length;

    //TODO 发送消息
    if(this->t == LOGIN){
        GoBang::GoBangRequest req;
        req.set_type(GoBang::LOGIN);
        GoBang::LoginRequest *loginReq = req.mutable_loginreq();
        loginReq->set_name(name.toStdString());
        loginReq->set_passwd(passwd.toStdString());
        //序列化
        std::string s = req.SerializeAsString();
        //封包 发送
        memset(buf, 0, sizeof(buf));
        length = s.length();
        memcpy(buf, ptr, sizeof(int));
        memcpy(buf + 4, s.c_str(), length);
        n.socket.write(buf, length + 4);
    }
    else if(this->t == REGISTER){
        if(!regExpName.exactMatch(name)){
            info += "用户名只包含中文，英文字母和数字，长度3～10之间;\n";
        }
        if(!regExpPasswd.exactMatch(passwd)){
            info += "密码只包含字母、数字和下划线，长度在4~12之间；";
        }
        if(info != ""){
            QMessageBox::StandardButton result=QMessageBox::critical(this, "错误", info);
        }
        else{
            GoBang::GoBangRequest req;
            req.set_type(GoBang::REGISTER);
            GoBang::RegisterRequest *registerReq = req.mutable_registerreq();
            registerReq->set_name(name.toStdString());
            registerReq->set_passwd(passwd.toStdString());
            //序列化
            std::string s = req.SerializeAsString();
            //封包 发送
            memset(buf, 0, sizeof(buf));
            length = s.length();
            memcpy(buf, ptr, sizeof(int));
            memcpy(buf + 4, s.c_str(), length);
            n.socket.write(buf, length + 4);
        }
    }

}

/**
 * @brief socket消息读取
 */
void MainWindow::readReady(){
    char len[4];
    char buf[4096];
    int length = 0;
    int *ptr = &length;
    std::vector<std::string> message;
    while(n.socket.bytesAvailable() > 4){
        memset(len, 0, sizeof(len));
        n.socket.read(len, 4);
        memcpy(ptr, len, sizeof(int));

        memset(buf, 0, sizeof(buf));
        n.socket.read(buf, std::min((int)sizeof(buf), length));
        message.emplace_back(std::string(buf));
    }

    GoBang::GoBangResponse goBangResponse;
    for(int i = 0; i < message.size(); i++){
        bool ret = goBangResponse.ParseFromString(message[i]);
        if(!ret){
            std::cout << "protobuf error\n";
        }
        GoBang::GoBangRequest req;
        switch(goBangResponse.type()){
        case(GoBang::LOGIN):{
            bool success = goBangResponse.loginresp().success();
            if(success){
                //登陆成功
                //socket转移给menu界面
                this->menu.moveNetControl();
                //将这个页面的socket解绑
                disconnect(&n.socket, &QTcpSocket::readyRead, this, &MainWindow::readReady);
                disconnect(&n.socket, &QTcpSocket::connected, this, &MainWindow::connected);
                disconnect(&n.socket, &QTcpSocket::disconnected, this, &MainWindow::disconnected);

                std::string uid = goBangResponse.loginresp().uid();
                //请求玩家数据
                //填充数据
                req.set_type(GoBang::PLAYER_MESSAGE);
                GoBang::PlayerMessageRequest *playerReq = req.mutable_playmessagereq();
                playerReq->set_uid(uid);

                //序列化
                std::string s = req.SerializeAsString();
                //封包 发送
                memset(buf, 0, sizeof(buf));
                length = s.length();
                memcpy(buf, ptr, sizeof(int));
                memcpy(buf + 4, s.c_str(), length);
                n.socket.write(buf, length + 4);

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
                n.socket.write(buf, length + 4);

                //显示menu窗口
                this->menu.show();
                //关闭登陆窗口
                this->close();

                bool isReconnect = goBangResponse.loginresp().isreconnect();
                if(isReconnect){
                    //如果是重连用户需要直接发送joinroom
                    std::string rid = goBangResponse.loginresp().rid();
                    req.set_type(GoBang::JOIN_ROOM);
                    GoBang::JoinRoomRequest *joinRoomRequest = req.mutable_joinroomreq();
                    joinRoomRequest->set_uid(uid);
                    joinRoomRequest->set_rid(rid);

                    //序列化
                    std::string s = req.SerializeAsString();
                    //封包 发送
                    memset(buf, 0, sizeof(buf));
                    length = s.length();
                    memcpy(buf, ptr, sizeof(int));
                    memcpy(buf + 4, s.c_str(), length);
                    n.socket.write(buf, length + 4);
                }

            }
            else{
                //登陆失败
                QMessageBox::StandardButton result=QMessageBox::critical(this, "错误", QString::fromStdString(goBangResponse.loginresp().msg()));
            }
            break;
        }
        case(GoBang::REGISTER):{
            bool success = goBangResponse.registerresp().success();
            if(success){
                //注册成功
                QMessageBox::StandardButton result=QMessageBox::information(this, "信息", "注册成功");
                this->switchButton();
            }
            else{
                //注册失败
                QMessageBox::StandardButton result=QMessageBox::critical(this, "错误", QString::fromStdString(goBangResponse.registerresp().msg()));
            }
            break;
        }
        default:
            break;
        }
    }


//    std::cout << "avliable " <<  n.socket.bytesAvailable() << '\n';
//    char buf[1024];
//    memset(buf, 0, sizeof(buf));
//    std::cout << "read:" << n.socket.read(buf, 4) << '\n';
//    int length = 0;
//    int *ptr = &length;
//    memcpy(ptr, buf, sizeof(int));
//    std::cout << length << '\n';
}

void MainWindow::connected(){
    this->show();
}

void MainWindow::disconnected(){
    menu.close();
    this->close();
}


