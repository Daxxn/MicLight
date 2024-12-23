/*
 * LightingManager.h
 *
 *  Created on: Dec 17, 2024
 *      Author: Daxxn
 */

#ifndef INC_LIGHTINGMANAGER_H_
#define INC_LIGHTINGMANAGER_H_

#include "main.h"
#include "Pins.h"
#include "AP33772Driver.h"

#define LED_COUNT 8
#define LED_DEFAULT_MAX_TEMP 60.0f
#define MCU_DEFAULT_MAX_TEMP 55.0f
#define UINT16_MAXF 65535.0f
#define UINT12_MAXF 04095.0f
#define LED_PWR_DERATE_5V  0.65f
#define LED_PWR_DERATE_9V  0.80f
#define LED_PWR_DERATE_12V 1.00f

typedef struct {
	float v5  = 0.65f;
	float v9  = 0.8f;
	float v12 = 1.0f;
} LightingSupplyDeratings;

typedef struct {
	Pin status;
	Pin maxBright;
	Pin struggleBus;
} LightingIndicators;

/*
 *
 */
class LightingManager
{
public:
	LightingManager() { };
	LightingManager(AP33772Driver *pdDriver, PWMPin *pins, LightingIndicators *indPins, AP33772_PDRequestObject *desiredPDO);
	virtual ~LightingManager();

	HAL_StatusTypeDef Init();

	void StatusInterrupt();

	void LightsON();
	void LightsOFF();
	void LightsToggle();
	void SetBrightness(float bright);
	void SetPotBrightness(uint16_t bright);
	void SetMcuTemperature(uint16_t temp);
	void ToggleMaxBrightness();
	void SetManualDerate(float derate);
	void SetSelectedPDO(AP33772_PDObject *selectedPDO);
	void UseTempDerate();
	void SetMaxTemperature(float maxTemp);
	void SetMeasurementsPtr(AP33772_MeasuredValues *measurements);

	void Update();
private:
	AP33772Driver           *PdDriver;
	PWMPin                  *Pins;
	LightingIndicators      *IndPins;
	AP33772_PDObject        *SelectedPDO;
	AP33772_PDRequestObject *DesiredPDO;
	AP33772_MeasuredValues  *MeasuredValues;

	float    McuTemp         = 0;
	uint16_t RawBrightness   = 0;
	uint16_t RawMaxBright    = 0;
	float    Brightness      = 0;
	uint16_t PotPosition     = 0;
	uint16_t PrevPotPosition = 0;
	bool     Enable          = false;
	bool     MaxBrightMode   = false;
	float    ManualDerate    = 0;
	float    Derate          = 0;
	float    TempDerateScale = 3.0f;
	float    SupplyDerate    = 1.0f;
	float    MaxTemperature  = LED_DEFAULT_MAX_TEMP;

	bool     UpdateFlag      = false;

	void CalcSupplyDerate();
	void CalcDerate();
	void SetLEDs();

	static float InvertScale(float input);
};

#endif /* INC_LIGHTINGMANAGER_H_ */
