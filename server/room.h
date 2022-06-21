//
// Created by chen on 2022/6/20.
//

#ifndef SERVER_ROOM_H
#define SERVER_ROOM_H
#include "player.h"

class room {
public:
	room(const std::string &roomId, const player &p1, const player &p2);
	room();
	virtual ~room();

	const std::string &getRoomId() const;

	void setRoomId(const std::string &roomId);

	const player &getP1() const;

	void setP1(const player &p1);

	const player &getP2() const;

	void setP2(const player &p2);

	bool anotherPlayer(player &, player &);

	bool operator==(const room &rhs) const;

	bool operator!=(const room &rhs) const;

private:

public:
	bool mask[3];   //0 UNDO 1 TIE 2 SURRENDER
	std::string room_id;
	player p1;
	player p2;
};


#endif //SERVER_ROOM_H
