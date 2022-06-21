//
// Created by chen on 2022/6/20.
//

#include "player.h"

player::player() {

}

player::player(int fd, std::string &nickname) {
	this->fd = fd;
	this->nickname = nickname;
}

player::~player() {}

int player::getFd() const {
	return fd;
}

void player::setFd(int fd) {
	player::fd = fd;
}

const std::string &player::getNickname() const {
	return nickname;
}

void player::setNickname(const std::string &nickname) {
	player::nickname = nickname;
}

bool player::operator==(const player &rhs) const {
	return fd == rhs.fd &&
	       nickname == rhs.nickname;
}

bool player::operator!=(const player &rhs) const {
	return !(rhs == *this);
}
