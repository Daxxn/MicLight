/*
 * AP33772Driver.h
 *
 *  Created on: May 21, 2024
 *      Author: Daxxn
 */

#ifndef INC_AP33772DRIVER_H_
#define INC_AP33772DRIVER_H_

#include "main.h"
#include "AP33772Types.h"

/*
 *
 */
class AP33772Driver
{
public:
	AP33772Driver() { };
	AP33772Driver(I2C_HandleTypeDef *hi2c);
	AP33772Driver(I2C_HandleTypeDef *hi2c, AP33772_PDRequestObject *desiredPDO);
	virtual ~AP33772Driver();

	HAL_StatusTypeDef Init();

	void StatusInterrupt();
	void FaultIndInterrupt();

	HAL_StatusTypeDef GetStatus();

	HAL_StatusTypeDef GetVoltage();
	HAL_StatusTypeDef GetCurrent();
	HAL_StatusTypeDef GetTemp();
	HAL_StatusTypeDef GetMeasurements();

	AP33772_MeasuredValues *GetMeasuredValuesPtr();

	HAL_StatusTypeDef SendRequestedPDO();
	HAL_StatusTypeDef SetInterrupts(AP33772_Status enabledInts);
	HAL_StatusTypeDef SetOverTemp(uint8_t temp);
	HAL_StatusTypeDef SetThermistorValues();

	bool FoundPDOMatch() { return this->foundPDO; };
	bool GetVoltageMismatch() { return this->voltageMismatch; };
	AP33772_PDObject* GetSelectedPDO();

	AP33772_ThermValues* GetThermValues();
	float GetTemperature() { return this->measured.temp; };

	AP33772_Status *GetIntStatus() { return &this->status; };

private:
	I2C_HandleTypeDef *hi2c;
	uint8_t address;
	uint8_t buffer[AP33772_BUFFER_SIZE];
	uint8_t txBuffer[AP33772_TX_BUFFER_SIZE];
	uint32_t prevFaultTick;

	AP33772_MeasuredValues measured;

	AP33772_Status status;
	AP33772_Status enabledInterrupts;

	AP33772_PDObject srcPDOList[AP33772_PDO_LIST_SIZE];
	uint8_t srcPDOCount = 0;
	uint8_t selectedPDOIndex = 0; // the first PDO should always be a fixed 5v supply?
	AP33772_PDRequestObject *desiredPDO;
	bool foundPDO = false;
	bool voltageMismatch = false;
	bool newPDOFlag = false;

	AP33772_ThermValues thermValues = defaultThermValues;
	UInt32Union temp;

	AP33772_PDObject ParsePDO(uint8_t *buffer);

	void FindNearestPDO();

	HAL_StatusTypeDef ReadSourcePDOs();
	HAL_StatusTypeDef ReadRegister(AP33772_Register cmd, uint8_t len);
	HAL_StatusTypeDef WriteRegister(AP33772_Register cmd, uint8_t len);
};

#endif /* INC_AP33772DRIVER_H_ */
