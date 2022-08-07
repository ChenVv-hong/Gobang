#include "mainwindow.h"

#include <QApplication>
#include <mainwindow.h>
#include <net.h>


int main(int argc, char *argv[])
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    QApplication a(argc, argv);
    MainWindow *m = new MainWindow();
    if(!m->connectServer()){
        //BUG 服务器连接失败 程序无法完全退出
        delete m;
    }
//    test t;
//    t.show();
    int ret = a.exec();
    delete m;
    google::protobuf::ShutdownProtobufLibrary();
    return ret;
}
