/*
 * Utils.h
 *
 *  Created on: Feb 12, 2023
 *      Author: Daxxn
 */

#ifndef INC_UTILS_H_
#define INC_UTILS_H_

#include "stm32f0xx_hal_gpio_ex.h"

typedef enum {
	// Because ACTIVE_HIGH is assigned 0, it sets the default sigal low.
	ACTIVE_HIGH = 0,
	ACTIVE_LOW = 1,
} GPIO_Default_State;

#if __cplusplus

struct Pin
{
	// Defaults to output pin
	Pin() {};
	// Defaults to output pin
	Pin(GPIO_TypeDef* port, uint16_t pin)
	{
		this->port = port;
		this->pin = pin;
		this->defaultState = ACTIVE_HIGH;
	};
	Pin(GPIO_TypeDef* port, uint16_t pin, bool isOutput)
	{
		this->port = port;
		this->pin = pin;
		this->defaultState = ACTIVE_HIGH;
		this->isOutput = isOutput;
	};
	Pin(GPIO_TypeDef* port, uint16_t pin, GPIO_Default_State defaultState)
	{
		this->port = port;
		this->pin = pin;
		this->defaultState = defaultState;
		this->isOutput = true;
	};
	Pin(GPIO_TypeDef* port, uint16_t pin, GPIO_Default_State defaultState, bool isOutput)
	{
		this->port = port;
		this->pin = pin;
		this->defaultState = defaultState;
		this->isOutput = isOutput;
	};

	void Write(GPIO_PinState state) {
		if (this->isOutput)
		{
			this->state = state;
			HAL_GPIO_WritePin(this->port, this->pin, (GPIO_PinState)(state ^ this->defaultState));
		}
	};
	void Write(bool state) {
		this->Write((GPIO_PinState)state);
	};
	GPIO_PinState Read() {
		if (!this->isOutput)
		{
			this->state = (GPIO_PinState)(HAL_GPIO_ReadPin(this->port, this->pin) ^ this->defaultState);
		}
		return this->state;
	};
	bool Read(int &&_)
	{
		if (!this->isOutput)
		{
			return (bool)(this->Read());
		}
		return this->state;
	};
	void Toggle() {
		if (this->isOutput)
		{
			HAL_GPIO_TogglePin(this->port, this->pin);
			this->state = (GPIO_PinState)!this->state;
		}
	};
	void Clear() {
		if (this->isOutput)
		{
			this->state = (GPIO_PinState)this->defaultState;
			HAL_GPIO_WritePin(this->port, this->pin, (GPIO_PinState)this->defaultState);
		}
	};
	GPIO_PinState GetState() {
		return this->state;
	};
private:
	GPIO_TypeDef* port;
	uint16_t pin;
	GPIO_Default_State defaultState;
	GPIO_PinState state;
	bool isOutput = true;
};

#endif /* __cplusplus */
#endif /* INC_UTILS_H_ */
