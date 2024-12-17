/*
 * MainWrapper.cpp
 *
 *  Created on: Mar 30, 2024
 *      Author: Daxxn
 */

#include "main.h"
#include "Utils.h"
#include "LightingControl.h"
#include "usb_device.h"
#include "AP33772Driver.h"

ADC_HandleTypeDef *h_adc;

TIM_HandleTypeDef *h_tim2;
TIM_HandleTypeDef *h_tim3;
TIM_HandleTypeDef *h_tim_debounce;
TIM_HandleTypeDef *h_tim_temp;

I2C_HandleTypeDef *h_i2c;

PCD_HandleTypeDef *h_usb;

PWMPin pins[LED_COUNT] = {};

AP33772Driver pwrDriver;

uint16_t potPosition = 0;
uint8_t pcbTemp = 0;
uint8_t overTempValue = OVERTEMP_DEFAULT;
uint8_t normTempValue = NORMAL_TEMP_DEFAULT;

//Pin enablePin = Pin(ENABLE_GPIO_Port, ENABLE_Pin, ACTIVE_LOW, false);
Pin maxBrightPin = Pin(MAX_BRIGHT_GPIO_Port, MAX_BRIGHT_Pin, ACTIVE_LOW, false);
Pin maxBrightIndPin = Pin(MAX_BRIGHT_IND_GPIO_Port, MAX_BRIGHT_IND_Pin);
Pin statusIndPin = Pin(STATUS_IND_GPIO_Port, STATUS_IND_Pin);
Pin underPwrPin = Pin(UNDER_PWR_IND_GPIO_Port, UNDER_PWR_IND_Pin);

bool lightsEnable = false;
bool maxBrightEnable = false;

bool debounce = false;
bool checkTemp = false;
bool overTemp = false;
bool derateTemp = false;

double temp = 0;

AP33772_PDRequestObject desiredPDO = {
		.maxCurrent = 2.8,
		.current = 2,
		.voltage = 12,
		.pdoType = AP33772_FIXED_PDO,
};

void OverTempCheck()
{
	pcbTemp = pwrDriver.GetMeasuredValues()->temp;
	if (!lightsEnable) return;
	if (pcbTemp > normTempValue) {
		derateTemp = true;
	} else {
		derateTemp = false;
	}
	if (pcbTemp > overTempValue) {
		overTemp = true;
		derateTemp = false;
	}
	if (overTemp && pcbTemp < normTempValue) {
		overTemp = false;
	}
}

void UsbReceiveCallback(uint8_t *buffer, uint32_t *len)
{
//	usbManager.UsbReceivedCallback(buffer, len);
}

void StartDebounce()
{
	debounce = true;
	HAL_TIM_Base_Start_IT(h_tim_debounce);
}

// Not sure how well this will work.
void CalcOverTempDerate()
{
	double tempOffset = (pcbTemp - normTempValue) / overTempValue;
	if (tempOffset < 0) return;
	temp -= tempOffset;
	if (temp < 0) temp = 0;
	else if (temp > 1) temp = 1;
}

void ControlLighting()
{
	if (!lightsEnable) {
		SetAllLights(0);
		return;
	}

	if (maxBrightEnable) {
		SetMaxBrightness();
		return;
	}

	temp = (double)potPosition / UINT12_MAXF;
	if (overTemp) {
		temp = temp / 0.8;
	} else if (derateTemp) {
		CalcOverTempDerate();
	}
	SetAllLights((uint16_t)(temp * UINT16_MAX));

}

void ADCConvCallback()
{
	potPosition = (uint16_t)HAL_ADC_GetValue(h_adc);
}

void ExtInterruptCallback(uint16_t pin)
{
	switch (pin) {
		case ENABLE_Pin:
			lightsEnable = !lightsEnable;
//			if (!debounce) {
//				lightsEnable = !lightsEnable;
//				StartDebounce();
//			}
			break;
		case I2C_INT_Pin:
			pwrDriver.HandleInterrupt();
		case USB_OVER_VOLT_Pin:
			lightsEnable = false;
			break;
		default:
			break;
	}
}

void TimerElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim == h_tim_debounce) {
		debounce = false;
	}
	else if (htim == h_tim_temp) {
		checkTemp = true;
	}
}

HAL_StatusTypeDef Init(
		ADC_HandleTypeDef *hadc,
		TIM_HandleTypeDef *htim2,
		TIM_HandleTypeDef *htim3,
		TIM_HandleTypeDef *htim7,
		TIM_HandleTypeDef *htim14,
		I2C_HandleTypeDef *hi2c2
	)
{
	h_adc          = hadc;
	h_tim2         = htim2;
	h_tim3         = htim3;
	h_tim_debounce = htim7;
	h_tim_temp     = htim14;
	h_i2c          = hi2c2;

	pwrDriver = AP33772Driver(h_i2c, desiredPDO);

	if (pwrDriver.Init() != HAL_OK) {
		return HAL_ERROR;
	}

	// Some kind of problem during startup. possibly an issue with the status interrupt?
	// The MCU needs to be reset for the power setup to work. This delay isnt enough.
	HAL_Delay(100);

	if (pwrDriver.SendRequestedPDO() != HAL_OK) {
		return HAL_ERROR;
	}

	if (pwrDriver.SetOverTemp(overTempValue) != HAL_OK) {
		return HAL_ERROR;
	}

	underPwrPin.Write(pwrDriver.GetVoltageMismatch());

	PWMPin ch1 = { .ch = TIM_CHANNEL_4, .handle = h_tim2 };
	PWMPin ch2 = { .ch = TIM_CHANNEL_1, .handle = h_tim3 };
	PWMPin ch3 = { .ch = TIM_CHANNEL_2, .handle = h_tim3 };
	PWMPin ch4 = { .ch = TIM_CHANNEL_3, .handle = h_tim3 };
	PWMPin ch5 = { .ch = TIM_CHANNEL_4, .handle = h_tim3 };
	PWMPin ch6 = { .ch = TIM_CHANNEL_1, .handle = h_tim2 };
	PWMPin ch7 = { .ch = TIM_CHANNEL_2, .handle = h_tim2 };
	PWMPin ch8 = { .ch = TIM_CHANNEL_3, .handle = h_tim2 };

	pins[0] = ch1;
	pins[1] = ch2;
	pins[2] = ch3;
	pins[3] = ch4;
	pins[4] = ch5;
	pins[5] = ch6;
	pins[6] = ch7;
	pins[7] = ch8;

	if (InitLighting(pins) != HAL_OK) {
		return HAL_ERROR;
	}

	if (HAL_TIM_Base_Start_IT(h_tim_temp) != HAL_OK) {
		return HAL_ERROR;
	}

	return HAL_OK;
}

void Main()
{
	if (maxBrightPin.Read(0)) {
		maxBrightEnable = !maxBrightEnable;
//		if (!debounce) {
//			maxBrightEnable = !maxBrightEnable;
//			StartDebounce();
//		}
	}

	if (HAL_ADC_Start(h_adc) == HAL_OK) {
		HAL_ADC_PollForConversion(h_adc, 100);
		potPosition = (uint16_t)HAL_ADC_GetValue(h_adc);
	}

	ControlLighting();

	maxBrightIndPin.Write(maxBrightEnable);
	statusIndPin.Write(lightsEnable);

	if (checkTemp) {
		pwrDriver.GetTemp();
		checkTemp = false;
		OverTempCheck();
	}

	HAL_Delay(1);
}
