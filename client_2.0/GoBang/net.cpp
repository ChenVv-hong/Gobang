#include "net.h"



Net::Net(QString ip, int port)
{
    this->IP = ip;
    this->PORT = port;
}

Net::~Net(){

}

bool Net::connect(){
    this->socket.connectToHost(this->IP, this->PORT);
//    return true;
    int ret = this->socket.waitForConnected();
    return ret;
}
void Net::disconnect(){
    if(this->socket.isOpen()){
        this->socket.disconnectFromHost();
        this->socket.waitForDisconnected();
    }
}
void Net::send(QString s){
    this->socket.waitForBytesWritten();
    this->socket.write(s.toStdString().c_str(),strlen(s.toStdString().c_str()));
//    this->socket.flush();
}
QString Net::read(){

    return QString(this->socket.readAll());
}
