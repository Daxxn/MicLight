/*
 * Pins.h
 *
 *  Created on: Dec 17, 2024
 *      Author: Daxxn
 */

#ifndef INC_PINS_H_
#define INC_PINS_H_

#ifdef STM32F412Vx
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_def.h"
#endif

#ifdef STM32F413xx
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_def.h"
#endif

#ifdef STM32F303xC
#include "stm32f3xx_hal.h"
#include "stm32f3xx_hal_def.h"
#endif

#ifdef STM32F072xB
#include "stm32f0xx_hal.h"
#endif

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
		if (this->isOutput && this->state != state)
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

struct PWMPin
{
public:
	PWMPin() { };
	PWMPin(TIM_HandleTypeDef* timerHandle, uint16_t channel) {
		this->timerHandle = timerHandle;
		this->channel = channel;
	};
	HAL_StatusTypeDef Init() {
		return HAL_TIM_PWM_Start(this->timerHandle, this->channel);
	};
	HAL_StatusTypeDef DeInit() {
		return HAL_TIM_PWM_Stop(this->timerHandle, this->channel);
	};
	void SetPWM(uint16_t value) {
		__HAL_TIM_SET_COMPARE(this->timerHandle, this->channel, value);
	};
	void ResetPWM() {
		__HAL_TIM_SET_COMPARE(this->timerHandle, this->channel, 0);
	};
private:
	TIM_HandleTypeDef* timerHandle;
	uint16_t channel = TIM_CHANNEL_1;
};

#endif /* __cplusplus */
#endif /* INC_PINS_H_ */
