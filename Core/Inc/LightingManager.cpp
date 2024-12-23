/*
 * LightingManager.cpp
 *
 *  Created on: Dec 17, 2024
 *      Author: Daxxn
 */

#include <LightingManager.h>
#include "math.h"

LightingManager::LightingManager(AP33772Driver *pdDriver, PWMPin *pins, LightingIndicators *indPins, AP33772_PDRequestObject *desiredPDO)
{
	PdDriver   = pdDriver;
	Pins       = pins;
	IndPins    = indPins;
	DesiredPDO = desiredPDO;
}

LightingManager::~LightingManager()
{
	PdDriver   = nullptr;
	Pins       = nullptr;
	IndPins    = nullptr;
	DesiredPDO = nullptr;
}

HAL_StatusTypeDef LightingManager::Init()
{
	for (int i = 0; i < LED_COUNT; ++i) {
		if (Pins[i].Init() != HAL_OK) {
			return HAL_ERROR;
		}
	}
	UpdateFlag = true;

	return HAL_OK;
}

void LightingManager::StatusInterrupt()
{
	PdDriver->StatusInterrupt();
}

void LightingManager::LightsON()
{
	Enable     = true;
	UpdateFlag = true;
}

void LightingManager::LightsOFF()
{
	Enable     = false;
	UpdateFlag = true;
}

void LightingManager::LightsToggle()
{
	Enable     = !Enable;
	UpdateFlag = true;
}

void LightingManager::SetBrightness(float bright)
{
	Brightness = bright;
	UpdateFlag = true;
}

void LightingManager::SetPotBrightness(uint16_t bright)
{
	PotPosition = bright;
	if (PotPosition > (bright - 1) && PotPosition < (bright + 1)) {
		Brightness      = (float)PotPosition / UINT12_MAXF;
		PrevPotPosition = PotPosition;
		UpdateFlag      = true;
	}
}

void LightingManager::SetMcuTemperature(uint16_t temp)
{
	McuTemp    = (float)temp / UINT12_MAXF;
	UpdateFlag = true;
}

void LightingManager::ToggleMaxBrightness()
{
	MaxBrightMode = !MaxBrightMode;
	UpdateFlag    = true;
}

void LightingManager::SetManualDerate(float derate)
{
	ManualDerate = derate;
	UpdateFlag   = true;
}

void LightingManager::SetSelectedPDO(AP33772_PDObject *selectedPDO)
{
	SelectedPDO = selectedPDO;
	if (SelectedPDO != nullptr) {
		CalcSupplyDerate();
		if (SelectedPDO->minVoltage)
		UpdateFlag    = true;
		if (SupplyDerate < 1.0f) {
			IndPins->struggleBus.Write(true);
		} else {
			IndPins->struggleBus.Write(false);
		}
	}
}

void LightingManager::UseTempDerate()
{
	ManualDerate = 0;
	UpdateFlag   = true;
}

void LightingManager::SetMaxTemperature(float maxTemp)
{
	MaxTemperature = maxTemp;
	UpdateFlag     = true;
}

void LightingManager::SetMeasurementsPtr(AP33772_MeasuredValues *measurements)
{
	MeasuredValues = measurements;
}

void LightingManager::CalcSupplyDerate()
{
	// Calc Voltage Derate:
	if (SelectedPDO->minVoltage < 6) {
		SupplyDerate = LED_PWR_DERATE_5V;
	} else if (SelectedPDO->minVoltage < 10) {
		SupplyDerate = LED_PWR_DERATE_9V;
	} else {
		SupplyDerate = LED_PWR_DERATE_12V;
	}

	// Calc Current Derate:
	SupplyDerate = SelectedPDO->current / DesiredPDO->maxCurrent;
}

void LightingManager::CalcDerate()
{
	// Manual Derating Mode:
	// Activated via the USB 'ManualDerate' command.
	if (ManualDerate != 0) {
		Derate = ManualDerate * SupplyDerate;
		if (MaxBrightMode) {
			Brightness = Derate;
			RawBrightness = Derate * UINT16_MAXF;
		} else {
			RawBrightness = Brightness * Derate * UINT16_MAXF;
			RawMaxBright = Derate * UINT16_MAXF;
		}
	} else {
//		Derate = std::pow(PwrTemp / LED_DEFAULT_MAX_TEMP, TempDerateScale) * SupplyDerate * (McuTemp / MCU_DEFAULT_MAX_TEMP);
		Derate = std::pow(MeasuredValues->temp / LED_DEFAULT_MAX_TEMP, TempDerateScale) * SupplyDerate;
		RawMaxBright = Derate * UINT16_MAXF;
		RawBrightness = Brightness * InvertScale(Derate) * UINT16_MAXF;
	}
}

void LightingManager::SetLEDs()
{
	if (Enable) {
		for (uint8_t i = 0; i < LED_COUNT; ++i) {
			Pins[i].SetPWM(RawBrightness);
		}
	} else {
		for (uint8_t i = 0; i < LED_COUNT; ++i) {
			Pins[i].SetPWM(0);
		}
	}
}

void LightingManager::Update()
{
	if (!UpdateFlag) return;
	if (SelectedPDO == nullptr) return;

	if (MeasuredValues->current >= SelectedPDO->current) {
		LightsOFF();
	}

	CalcDerate();
	SetLEDs();

	IndPins->status.Write(Enable);
	IndPins->maxBright.Write(MaxBrightMode);

	UpdateFlag = false;
}

float LightingManager::InvertScale(float input)
{
	return abs(input - 1);
}
