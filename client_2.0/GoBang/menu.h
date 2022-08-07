#ifndef MENU_H
#define MENU_H

#include <QMainWindow>
#include <QDebug>
#include <gameinterface.h>
#include <net.h>
#include <myprotocol.pb.h>
#include <QVector>
#include <playeritem.h>
#include <matchwait.h>

namespace Ui {
class Menu;
}

class Menu : public QMainWindow
{
    Q_OBJECT

public:
    explicit Menu(Net *n,QWidget *parent = nullptr);
    ~Menu();

    void moveNetControl();
    GoBang::Player& getMe();
    void setMe(GoBang::Player& p);
    void setRankList(QVector<GoBang::Player> &v);

private slots:
    void pressButtonNormal();
    void pressButtonRank();
    void pressButtonCreateRoom();
    void pressButtonJoinRoom();

    void readReady();
    void disconnected();


private:
    Ui::Menu *ui;
    Net *n;
    GoBang::Player me;
    GameInterface gameInterface;
    matchwait waitcreen;
};

#endif // MENU_H
