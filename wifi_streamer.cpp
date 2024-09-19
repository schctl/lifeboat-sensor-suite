#include <Wire.h>
#include <stdint.h>

// --------------------------------------------------

#ifndef INC_COM_HPP_
#define INC_COM_HPP_

#include <stdint.h>
#include <string.h>

// STM -> ESP messages

enum UserStatus {
	Ok = 1,
	Unhealthy = 2,
	Critical = 3,
	Unknown = 4
};

typedef struct {
	uint8_t buf[8];
} Ping;

// ESP -> STM messages

typedef struct {
	uint8_t bpm;
} BpmReading;

// ----------- Generic Message Ser/De implementation ----------------

union MessageType {
	// STM -> ESP messages
	UserStatus status;
	Ping ping;
	// ESP -> STM messages
	BpmReading bpm;
};

enum MessageTypeIdx {
	// STM -> ESP messages
	UserStatusIdx = 1,
	PingIdx = 2,
	// ESP -> STM messages
	BpmReadIdx = 3,
};

uint8_t TX_BUF[64] = { 0 };
uint8_t RX_BUF[64] = { 0 };

class Message {
public:
	MessageTypeIdx type;
	MessageType message;

	// -----------------------------------------------

	Message(Ping ping)
		: type(MessageTypeIdx::PingIdx), message(MessageType { .ping = ping }) {}

	Message(UserStatus status)
		: type(MessageTypeIdx::UserStatusIdx), message(MessageType { .status = status }) {}

	Message(BpmReading bpm)
			: type(MessageTypeIdx::BpmReadIdx), message(MessageType { .bpm = bpm }) {}

	// default constructor

	Message() : type(MessageTypeIdx::UserStatusIdx), message(MessageType { .status = UserStatus::Unknown }) {}

	// ------------------------------------------------

	void send(void) {
		uint16_t bytes = this->serialize(TX_BUF + 1);
		TX_BUF[0] = bytes;
		Serial1.write(TX_BUF, TX_BUF[0] + 1);
	}

	Message receive(void) {
		uint8_t len = 0;
		Message msg;

    /*
		RX_STATUS_LEN = HAL_UART_Receive(huart, &len, 1, 100);
		if (RX_STATUS_LEN == HAL_OK)
		{
			RX_STATUS_MSG = HAL_UART_Receive(huart, RX_BUF, len, 1000);

			if (RX_STATUS_MSG == HAL_OK) {
				msg = Message::deserialize(RX_BUF);
				memset(RX_BUF, 0, len);
			}
		}
    */

		return msg;
	}

	static Message deserialize(uint8_t* buffer) {
	    uint16_t ptr = 0;
	    MessageType msg;

	    MessageTypeIdx command = (MessageTypeIdx)(buffer[ptr]); ptr++;

	    switch (command) {
	    // STM -> ESP messages
	    case MessageTypeIdx::PingIdx:
	      memcpy(&msg.ping.buf, buffer + ptr, 8); ptr += 8;
	      return Message(msg.ping);
	    case MessageTypeIdx::UserStatusIdx:
	      msg.status = (UserStatus)(buffer[ptr]); ptr++;
	      return Message(msg.status);
	    // ESP -> STM messages
	    case MessageTypeIdx::BpmReadIdx:
	    	msg.bpm = BpmReading { .bpm = buffer[ptr] }; ptr++;
	    	return Message(msg.bpm);
	    }

	    msg.status = UserStatus::Unknown;
	    return Message(msg.status);
	  }

private:
	uint16_t serialize(uint8_t* buf) {
		uint16_t ptr = 0;

		buf[ptr] = (uint8_t)(this->type); ptr++;

		switch (this->type) {
		// STM -> ESP messages
		case MessageTypeIdx::PingIdx:
			memcpy(buf + ptr, this->message.ping.buf, 8); ptr += 8;
		case MessageTypeIdx::UserStatusIdx:
			buf[ptr] = (uint8_t)(this->message.status); ptr++;
		// ESP -> STM messages
		case MessageTypeIdx::BpmReadIdx:
			buf[ptr] = this->message.bpm.bpm; ptr++;
		}

		return ptr;
	}
};

#endif /* INC_COM_HPP_ */

// --------------------------------------------------

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while (!Serial);

  Serial1.begin(115200);
  while (!Serial1);
}

void recvUserStatus() {
  uint8_t buf[64] = { 0 };

  while (Serial1.available()) {
    uint8_t len = Serial1.read();
    Serial1.readBytes(buf, len);

    for (int i = 0; i < len; i++) {
      Serial.print(buf[i]);
      Serial.print(" ");
    }
    Serial.println();
  }

  Message msg = Message::deserialize(buf);
}

void loop() {
  recvUserStatus();
  delay(100);

}
