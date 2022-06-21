#ifndef PLAYER_H
#define PLAYER_H

#include <QString>

class Player
{
public:
    Player();
    Player(QString a);
    QString getNickName();
    void setNickName(QString a);

private:
    QString nickname;
};

#endif // PLAYER_H
