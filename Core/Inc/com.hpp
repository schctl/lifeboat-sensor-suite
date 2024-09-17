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

#define WIFI_BOARD 7

enum UserStatus {
	Ok = 1,
	Unhealthy = 2,
	Critical = 3,
	Unknown = 4
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

	HAL_StatusTypeDef send(I2C_HandleTypeDef* hi2c) {
		HAL_StatusTypeDef status = HAL_OK;
		uint16_t dev_addr = (uint16_t)(WIFI_BOARD) << 7;

		status = HAL_I2C_IsDeviceReady(hi2c, dev_addr, 5, 100);

		if (status == HAL_OK) {
			uint16_t bytes = this->serialize();
			status = HAL_I2C_Master_Transmit(hi2c, dev_addr, TX_BUF, bytes, 1000);
		}

		return status;
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
