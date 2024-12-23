/*
 * LightingControl.cpp
 *
 *  Created on: Mar 30, 2024
 *      Author: Daxxn
 */

#include <LightingControl.h>

//LightingState state;
//uint8_t i = 0;
//double tempp = 0;
//
//void SetPWM(uint8_t i, uint16_t pwm)
//{
////	if (state.pins[i].is32BTimer) {
////		tempp = (double)pwm / UINT16_MAXF;
////		__HAL_TIM_SET_COMPARE(state.pins[i].handle, state.pins[i].ch, (uint32_t)(tempp * UINT32_MAXF));
////	}
////	else {
////		__HAL_TIM_SET_COMPARE(state.pins[i].handle, state.pins[i].ch, pwm);
////	}
//	__HAL_TIM_SET_COMPARE(state.pins[i].handle, state.pins[i].ch, pwm);
//}
//
//HAL_StatusTypeDef InitLighting(PWMPin *pins)
//{
//	state.pins = pins;
//	for (i = 0; i < LED_COUNT; ++i) {
//		if (HAL_TIM_PWM_Start(state.pins[i].handle, state.pins[i].ch) != HAL_OK) {
//			return HAL_ERROR;
//		}
//	}
//	LightsOff();
//	return HAL_OK;
//}
//
//void DeInitLighting()
//{
//	for (i = 0; i < LED_COUNT; ++i) {
//		HAL_TIM_PWM_Stop(state.pins[i].handle, state.pins[i].ch);
//	}
//	state.pins = NULL;
//}
//
//void LightsOff()
//{
//	SetAllLights(0);
//}
//
//void LightsOn()
//{
//	SetAllLights(0xFFFF);
//}
//
//void SetLight(uint8_t index, uint16_t pwm)
//{
//	assert_param(index < LED_COUNT);
//	SetPWM(index, pwm);
//}
//
//void SetAllLights(uint16_t pwm)
//{
//	for (i = 0; i < LED_COUNT; ++i) {
//		SetPWM(i, pwm);
//	}
//}
//
//void SetMaxBrightness()
//{
//	SetAllLights(UINT16_MAX);
//}
