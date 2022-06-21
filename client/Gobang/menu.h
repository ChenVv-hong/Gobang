#ifndef MENU_H
#define MENU_H

#include <QMainWindow>
#include <player.h>
#include <QDebug>
#include <gameinterface.h>

namespace Ui {
class Menu;
}

class Menu : public QMainWindow
{
    Q_OBJECT

public:
    explicit Menu(QWidget *parent = nullptr);
    ~Menu();

    void setPlayer(Player p);

private slots:
    void pressButtonMM();
    void pressButtonCM();
    void pressButtonCR();
    void pressButtonJR();

private:
    Ui::Menu *ui;
    Player me;
};

#endif // MENU_H
