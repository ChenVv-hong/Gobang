//
// Created by chen on 2022/6/20.
//

#ifndef SERVER_PLAYER_H
#define SERVER_PLAYER_H
#include <string>

class player {
public:
	player();
	player(int fd, std::string &nickname);
	~player();
	int getFd() const;

	void setFd(int fd);

	const std::string &getNickname() const;

	void setNickname(const std::string &nickname);

	bool operator==(const player &rhs) const;

	bool operator!=(const player &rhs) const;

private:
	int fd;
	std::string nickname;
};


#endif //SERVER_PLAYER_H
