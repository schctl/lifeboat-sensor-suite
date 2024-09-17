#include <Wire.h>
#include <stdint.h>

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

#define WIFI_BOARD 7

enum UserStatus {
	Ok = 1,
	Unhealthy = 2,
	Critical = 3,
  Unknown = 4,
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

  static Message deserialize(uint8_t* buffer) {
    uint16_t ptr = 0;
    MessageType msg;
  
    MessageTypeIdx command = (MessageTypeIdx)(buffer[ptr]); ptr++;

    switch (command) {
    case MessageTypeIdx::PingIdx:
      memcpy(&msg.ping.buf, buffer + ptr, 8); ptr += 8;
      return Message(msg.ping);
    case MessageTypeIdx::UserStatusIdx:
      msg.status = (UserStatus)(buffer[ptr]); ptr += 8;
      return Message(msg.status);
    }

    msg.status = UserStatus::Unknown;
    return Message(msg.status);
  }

private:
	uint16_t serialize() {
		memset(TX_BUF, 0, 64);

		uint16_t ptr = 0;

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

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while (!Serial);

  Wire.begin();
  Wire.onReceive(recvUserStatus);
}

void recvUserStatus(int bytes) {
  uint8_t buf[bytes] = { 0 };

  while (Wire.available()) {
    Wire.readBytes(buf, bytes);
  }

  Message msg = Message::deserialize(buf);
  Serial.println(msg.type);
}

void loop() {
  // put your main code here, to run repeatedly:

  Serial.println("heartbeat");
  delay(1000);

}
