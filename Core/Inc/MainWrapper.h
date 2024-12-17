/*
 * MainWrapper.h
 *
 *  Created on: Mar 30, 2024
 *      Author: Daxxn
 */

#ifndef INC_MAINWRAPPER_H_
#define INC_MAINWRAPPER_H_

void ADCConvCallback();
void ExtInterruptCallback(uint16_t pin);
void TimerElapsedCallback(TIM_HandleTypeDef *htim);

HAL_StatusTypeDef Init(
		ADC_HandleTypeDef *hadc,
		TIM_HandleTypeDef *htim2,
		TIM_HandleTypeDef *htim3,
		TIM_HandleTypeDef *htim7,
		TIM_HandleTypeDef *htim14,
		I2C_HandleTypeDef *hi2c2
	);

void Main();

#endif /* INC_MAINWRAPPER_H_ */
