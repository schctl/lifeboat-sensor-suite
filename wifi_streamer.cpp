#include <stdint.h>
#include <string.h>

UART mySerial(digitalPinToPinName(3), digitalPinToPinName(2));

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

uint8_t LEN = 0;

int RX_STATUS_LEN = 0;
int RX_STATUS_MSG = 0;

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

	int send() {
		uint16_t bytes = this->serialize(TX_BUF + 1);
		TX_BUF[0] = bytes;
		int status = mySerial.write(TX_BUF, bytes + 1);

		return status;
	}

	static Message receive() {
		LEN = 0;
		Message msg;

    if (mySerial.available()) {
      int bytes = mySerial.read();

      mySerial.readBytes(RX_BUF, bytes);
      msg = Message::deserialize(RX_BUF);
      memset(RX_BUF, 0, bytes);
    }

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
			break;
		case MessageTypeIdx::UserStatusIdx:
			buf[ptr] = (uint8_t)(this->message.status); ptr++;
			break;
		// ESP -> STM messages
		case MessageTypeIdx::BpmReadIdx:
			buf[ptr] = this->message.bpm.bpm; ptr++;
			break;
		}

		return ptr;
	}
};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while (!Serial);

  mySerial.begin(9600);
  while (!mySerial);

}

void loop() {
  // put your main code here, to run repeatedly:

    Serial.println(Message::receive().message.status);
  
  delay(100);
}
