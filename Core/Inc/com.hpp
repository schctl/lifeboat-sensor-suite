/*
 * com.hpp
 *
 *  Created on: Sep 16, 2024
 *      Author: sachin
 */

#ifndef INC_COM_HPP_
#define INC_COM_HPP_

#include <stdint.h>
#include <string.h>

enum UserStatus {
	Ok = 1,
	Unhealthy = 2,
	Critical = 3
};

typedef struct {
	uint8_t buf[8];
} Ping;

union MessageType {
	UserStatus status;
	Ping ping;
};

enum MessageTypeIdx {
	UserStatusIdx = 1,
	PingIdx = 2
};

uint8_t TX_BUF[64] = { 0 };
uint8_t RX_BUF[64] = { 0 };

class Message {
public:
	MessageTypeIdx type;
	MessageType message;

	Message(Ping ping)
		: type(MessageTypeIdx::PingIdx), message(MessageType { .ping = ping }) {}

	Message(UserStatus status)
		: type(MessageTypeIdx::UserStatusIdx), message(MessageType { .status = status }) {}

	uint8_t serialize() {
		memset(TX_BUF, 0, 64);

		uint8_t ptr = 0;

		TX_BUF[ptr] = (uint8_t)(this->type); ptr++;

		switch (this->type) {
		case MessageTypeIdx::PingIdx:
			memcpy(TX_BUF, this->message.ping.buf, 8); ptr += 8;
		case MessageTypeIdx::UserStatusIdx:
			TX_BUF[ptr] = (uint8_t)(this->message.status); ptr++;
		}

		return ptr;
	}
};

#endif /* INC_COM_HPP_ */
