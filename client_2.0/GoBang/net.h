#ifndef NET_H
#define NET_H

#include <QTcpSocket>
#include <string>
#include <iostream>


class Net
{
public:
    Net(QString ip, int port);
    ~Net();
    bool connect();
    void disconnect();
    void send(QString s);
    QString read();
public:
    QTcpSocket socket;
    QString IP;
    int PORT;

};

#endif // NET_H
