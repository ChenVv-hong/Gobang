#include "player.h"

Player::Player()
{
    this->nickname = "";
}

Player::Player(QString a)
{
    this->nickname = a;
}

void Player::setNickName(QString a){
    this->nickname = a;
}

QString Player::getNickName(){
   return this->nickname;
}


