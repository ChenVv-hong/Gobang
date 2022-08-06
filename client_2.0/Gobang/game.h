#ifndef GAME_H
#define GAME_H

#include "mainwindow.h"
#include "gameinterface.h"
#include "menu.h"

class Game
{
public:
    Game();
    void start();
private:
    MainWindow *login;
    Menu *menu;
    GameInterface *gameinterface;
};

#endif // GAME_H
