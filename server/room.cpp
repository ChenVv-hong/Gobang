//
// Created by chen on 2022/6/20.
//

#include "room.h"

room::room(const std::string &roomId, const player &p1, const player &p2) : room_id(roomId), p1(p1), p2(p2) {
	for(int i = 0; i < 3; i++){
		this->mask[i] = false;
	}
}

room::~room() {

}

const std::string &room::getRoomId() const {
	return room_id;
}

void room::setRoomId(const std::string &roomId) {
	room_id = roomId;
}

const player &room::getP1() const {
	return p1;
}

void room::setP1(const player &p1) {
	room::p1 = p1;
}

const player &room::getP2() const {
	return p2;
}

void room::setP2(const player &p2) {
	room::p2 = p2;
}


bool room::anotherPlayer(player &p, player &another) {
	if(p.getFd() == this->p1.getFd()){
		another = this->p2;
		return true;
	}else if(p.getFd() == this->p2.getFd()){
		another = this->p1;
		return true;
	}else{
		return false;
	}

}

room::room() {

}

bool room::operator==(const room &rhs) const {
	return room_id == rhs.room_id &&
	       p1 == rhs.p1 &&
	       p2 == rhs.p2 &&
	       mask == rhs.mask;
}

bool room::operator!=(const room &rhs) const {
	return !(rhs == *this);
}
