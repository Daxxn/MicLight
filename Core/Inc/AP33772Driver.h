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
	AP33772Driver(I2C_HandleTypeDef *hi2c, AP33772_PDRequestObject desiredPDO);
	virtual ~AP33772Driver();

	HAL_StatusTypeDef Init();

	void HandleInterrupt();

	HAL_StatusTypeDef GetStatus();

	HAL_StatusTypeDef GetVoltage();
	HAL_StatusTypeDef GetCurrent();
	HAL_StatusTypeDef GetTemp();

	AP33772_MeasuredValues *GetMeasuredValues();

	HAL_StatusTypeDef SendRequestedPDO();
	HAL_StatusTypeDef SetInterrupts(AP33772_Status enabledInts);
	HAL_StatusTypeDef SetOverTemp(uint8_t temp);

	bool GetVoltageMismatch() { return this->voltageMismatch; };

	void GetTempRes();
private:
	I2C_HandleTypeDef *hi2c;
	uint8_t address;
	uint8_t *buffer;

	AP33772_MeasuredValues measured;

	AP33772_StatusUnion status;
	AP33772_StatusUnion enabledInterrupts;

	AP33772_PDObject *srcPDOList;
	uint8_t srcPDOCount;
	uint8_t selectedPDOIndex = 0; // the first PDO should always be a fixed 5v supply?
	AP33772_PDRequestObject desiredPDO;
	bool foundPDO = false;
	bool voltageMismatch = false;

	AP33772_PDOUnion temp;

	AP33772_PDObject ParsePDO(uint8_t *buffer);

	void FindNearestPDO();

	HAL_StatusTypeDef ReadSourcePDOs();
	HAL_StatusTypeDef ReadRegister(AP33772_Register cmd, uint8_t len);
};

#endif /* INC_AP33772DRIVER_H_ */
