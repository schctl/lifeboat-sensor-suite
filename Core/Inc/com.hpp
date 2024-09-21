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

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_uart.h"

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

uint8_t LEN = 0;

HAL_StatusTypeDef RX_STATUS_LEN = HAL_OK;
HAL_StatusTypeDef RX_STATUS_MSG = HAL_OK;

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

	HAL_StatusTypeDef send(UART_HandleTypeDef* huart) {
		uint16_t bytes = this->serialize(TX_BUF + 1);
		TX_BUF[0] = bytes;
		HAL_StatusTypeDef status = HAL_UART_Transmit(huart, TX_BUF, bytes + 1, 1000);

		return status;
	}

	static Message receive(UART_HandleTypeDef* huart) {
		LEN = 0;
		Message msg;

		RX_STATUS_LEN = HAL_UART_Receive(huart, &LEN, 1, 100);
		if (RX_STATUS_LEN == HAL_OK)
		{
			RX_STATUS_MSG = HAL_UART_Receive(huart, RX_BUF, LEN, 1000);

			if (RX_STATUS_MSG == HAL_OK) {
				msg = Message::deserialize(RX_BUF);
				memset(RX_BUF, 0, LEN);
			}
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

#endif /* INC_COM_HPP_ */
