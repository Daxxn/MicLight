/*
 * MainWrapper.cpp
 *
 *  Created on: Mar 30, 2024
 *      Author: Daxxn
 */

#include "main.h"
#include "Pins.h"
#include "LightingManager.h"
#include "usb_device.h"
#include "AP33772Driver.h"
#include "UsbManager.h"

// Peripheral Handle Pointers:
ADC_HandleTypeDef *h_adc;
TIM_HandleTypeDef *h_tim2;
TIM_HandleTypeDef *h_tim3;
TIM_HandleTypeDef *h_tim_debounce;
TIM_HandleTypeDef *h_tim_temp;
TIM_HandleTypeDef *h_tim_rdData;
TIM_HandleTypeDef *h_tim_pwrFault;
I2C_HandleTypeDef *h_i2c;
PCD_HandleTypeDef *h_usb;

// Private Variables:
PWMPin pins[LED_COUNT] = {};

AP33772Driver pwrDriver;
LightingManager lighting;
UsbManager usb;

uint16_t potPosition = 0;
uint8_t pcbTemp = 0;
uint8_t overTempValue = LED_DEFAULT_MAX_TEMP;
uint8_t normTempValue = NORMAL_TEMP_DEFAULT;

typedef struct {
	Pin enable;
	Pin maxBright;
} ControlPins;
ControlPins ctrlPins;
//Pin enablePin = Pin(ENABLE_GPIO_Port, ENABLE_Pin, ACTIVE_LOW, false);
LightingIndicators indPins;
//Pin maxBrightPin = Pin(MAX_BRIGHT_GPIO_Port, MAX_BRIGHT_Pin, ACTIVE_LOW, false);
//Pin maxBrightIndPin = Pin(MAX_BRIGHT_IND_GPIO_Port, MAX_BRIGHT_IND_Pin);
//Pin statusIndPin = Pin(STATUS_IND_GPIO_Port, STATUS_IND_Pin);
//Pin underPwrPin = Pin(UNDER_PWR_IND_GPIO_Port, UNDER_PWR_IND_Pin);

bool debounce = false;
bool checkTemp = false;
bool readData = false;
bool startup = true;
//bool overTemp = false;
//bool derateTemp = false;

//double temp = 0;

AP33772_PDRequestObject desiredPDO = {
		.maxCurrent = 2.2,
		.current = 2,
		.voltage = 12,
		.pdoType = AP33772_ANY_PDO,
};

// private Function Definitions:
static void InitPins();
static HAL_StatusTypeDef InitPower();

// Callback Functions:

void UsbReceiveCallback(uint8_t *buffer, uint32_t *len)
{
	usb.ReceivedCallback(buffer, len);
}

void ADCConvCallback()
{
	potPosition = (uint16_t)HAL_ADC_GetValue(h_adc);
}

void ExtInterruptCallback(uint16_t pin)
{
	switch (pin) {
		case ENABLE_Pin:
			if (startup) {
				lighting.LightsOFF();
				return;
			}
			lighting.LightsToggle();
			break;
		case MAX_BRIGHT_Pin:
			if (startup) return;
			lighting.ToggleMaxBrightness();
			break;
		case I2C_INT_Pin:
			lighting.StatusInterrupt();
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
	else if (htim == h_tim_rdData) {
		readData = true;
	}
}

HAL_StatusTypeDef Init(
		ADC_HandleTypeDef *hadc,
		TIM_HandleTypeDef *htim2,
		TIM_HandleTypeDef *htim3,
		TIM_HandleTypeDef *htim7,
		TIM_HandleTypeDef *htim14,
		TIM_HandleTypeDef *htim15,
		TIM_HandleTypeDef *htim16,
		I2C_HandleTypeDef *hi2c2
	)
{
	h_adc          = hadc;
	h_tim2         = htim2;
	h_tim3         = htim3;
	h_tim_debounce = htim7;
	h_tim_temp     = htim14;
	h_tim_pwrFault = htim15;
	h_tim_rdData   = htim16;
	h_i2c          = hi2c2;

	RegisterUSBReceiveCallback(UsbReceiveCallback);

	InitPins();

	pwrDriver = AP33772Driver(h_i2c, &desiredPDO);

	lighting = LightingManager(&pwrDriver, pins, &indPins, &desiredPDO);
	lighting.SetMeasurementsPtr(pwrDriver.GetMeasuredValuesPtr());

	usb = UsbManager(GetUSBHandle());

	// Some kind of problem during startup. possibly an issue with the status interrupt?
	// The MCU needs to be reset for the power setup to work.
	HAL_Delay(2000);

	if (InitPower() != HAL_OK) {
		return HAL_ERROR;
	}

	if (lighting.Init() != HAL_OK) {
		return HAL_ERROR;
	}

	if (HAL_TIM_Base_Start_IT(h_tim_temp) != HAL_OK) {
		return HAL_ERROR;
	}

	if (HAL_TIM_Base_Start_IT(h_tim_rdData) != HAL_OK) {
		return HAL_ERROR;
	}

	if (pwrDriver.SetInterrupts(interruptMask) != HAL_OK) {
		return HAL_ERROR;
	}

	startup = false;
	return HAL_OK;
}

void Main()
{
	if (ctrlPins.maxBright.Read()) {
		lighting.ToggleMaxBrightness();
//		if (!debounce) {
//			maxBrightEnable = !maxBrightEnable;
//			StartDebounce();
//		}
	}

	if (HAL_ADC_Start(h_adc) == HAL_OK) {
		if (HAL_ADC_PollForConversion(h_adc, 100) == HAL_OK) {
			lighting.SetPotBrightness((uint16_t)HAL_ADC_GetValue(h_adc));
//			lighting.SetMcuTemperature((uint16_t)HAL_ADC_GetValue(h_adc));
		}
	}

//	if (!lightsEnable) {
//		lighting.LightsOFF();
//	}

//	if (maxBrightEnable) {
//		lighting.ToggleMaxBrightness();
//		return;
//	}

//	lighting.SetPotBrightness(potPosition);

	if (checkTemp) {
//		pwrDriver.GetMeasurements();
		pwrDriver.GetTemp();
		checkTemp = false;
	}
	if (readData) {
		pwrDriver.GetMeasurements();
		readData = false;
	}

	lighting.Update();
}

void InitPins()
{
	PWMPin ch1 = PWMPin(h_tim2, TIM_CHANNEL_4);
	PWMPin ch2 = PWMPin(h_tim3, TIM_CHANNEL_1);
	PWMPin ch3 = PWMPin(h_tim3, TIM_CHANNEL_2);
	PWMPin ch4 = PWMPin(h_tim3, TIM_CHANNEL_3);
	PWMPin ch5 = PWMPin(h_tim3, TIM_CHANNEL_4);
	PWMPin ch6 = PWMPin(h_tim2, TIM_CHANNEL_1);
	PWMPin ch7 = PWMPin(h_tim2, TIM_CHANNEL_2);
	PWMPin ch8 = PWMPin(h_tim2, TIM_CHANNEL_3);

	pins[0] = ch1;
	pins[1] = ch2;
	pins[2] = ch3;
	pins[3] = ch4;
	pins[4] = ch5;
	pins[5] = ch6;
	pins[6] = ch7;
	pins[7] = ch8;

	indPins.maxBright   = Pin(MAX_BRIGHT_IND_GPIO_Port, MAX_BRIGHT_IND_Pin);
	indPins.status      = Pin(STATUS_IND_GPIO_Port, STATUS_IND_Pin);
	indPins.struggleBus = Pin(UNDER_PWR_IND_GPIO_Port, UNDER_PWR_IND_Pin);
	ctrlPins.enable     = Pin(ENABLE_GPIO_Port, ENABLE_Pin, ACTIVE_LOW, false);
	ctrlPins.maxBright  = Pin(MAX_BRIGHT_GPIO_Port, MAX_BRIGHT_Pin, ACTIVE_LOW, false);
}

HAL_StatusTypeDef InitPower()
{
	if (pwrDriver.Init() != HAL_OK) {
		for (int i = 0; i < 100; ++i) {
			indPins.status.Write(true);
			indPins.struggleBus.Write(true);
			if (pwrDriver.Init() == HAL_OK) {
				indPins.status.Write(false);
				indPins.struggleBus.Write(false);
				break;
			}
			HAL_Delay(1);
		}
	}

	if (pwrDriver.FoundPDOMatch()) {
		if (pwrDriver.SendRequestedPDO() != HAL_OK) {
			return HAL_ERROR;
		}

		if (pwrDriver.SetThermistorValues() != HAL_OK) {
			return HAL_ERROR;
		}

		if (pwrDriver.SetOverTemp(overTempValue) != HAL_OK) {
			return HAL_ERROR;
		}

		lighting.SetSelectedPDO(pwrDriver.GetSelectedPDO());
	}
	return HAL_OK;
}

//void OverTempCheck()
//{
	// OLD
//	pcbTemp = pwrDriver.GetMeasuredValuesPtr()->temp;
//	if (!lightsEnable) return;
//	if (pcbTemp > normTempValue) {
//		derateTemp = true;
//	} else {
//		derateTemp = false;
//	}
//	if (pcbTemp > overTempValue) {
//		overTemp = true;
//		derateTemp = false;
//	}
//	if (overTemp && pcbTemp < normTempValue) {
//		overTemp = false;
//	}
//}

//void StartDebounce()
//{
//	debounce = true;
//	HAL_TIM_Base_Start_IT(h_tim_debounce);
//}
//
//// Not sure how well this will work.
//void CalcOverTempDerate()
//{
//	double tempOffset = (pcbTemp - normTempValue) / overTempValue;
//	if (tempOffset < 0) return;
//	temp -= tempOffset;
//	if (temp < 0) temp = 0;
//	else if (temp > 1) temp = 1;
//}

//void ControlLighting()
//{
//	if (!lightsEnable) {
////		SetAllLights(0);
//		lighting.LightsOFF();
//		return;
//	}
//
//	if (maxBrightEnable) {
////		SetMaxBrightness();
//		lighting.ToggleMaxBrightness();
//		return;
//	}
//
//	lighting.SetPotBrightness(potPosition);
//
////	temp = (double)potPosition / UINT12_MAXF;
////	if (overTemp) {
////		temp = temp / 0.8;
////	} else if (derateTemp) {
////		CalcOverTempDerate();
////	}
////	SetAllLights((uint16_t)(temp * UINT16_MAX));
//
//}
