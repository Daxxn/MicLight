/*
 * AP33772Driver.cpp
 *
 *  Created on: May 21, 2024
 *      Author: Daxxn
 */

#include <AP33772Driver.h>
#include <math.h>

AP33772Driver::AP33772Driver(I2C_HandleTypeDef *hi2c)
{
	this->hi2c = hi2c;
	this->address = AP33772_ADDRESS << 1;
	this->buffer = new uint8_t[AP33772_BUFFER_SIZE];
	this->srcPDOList = new AP33772_PDObject[AP33772_PDO_LIST_SIZE];
	this->srcPDOCount = 0;
	this->temp.value = 0;
//	this->desiredPDO = defaultPDO;
}

AP33772Driver::AP33772Driver(I2C_HandleTypeDef *hi2c, AP33772_PDRequestObject desiredPDO)
{
	this->hi2c = hi2c;
	this->address = AP33772_ADDRESS << 1;
	this->buffer = new uint8_t[AP33772_BUFFER_SIZE];
	this->srcPDOList = new AP33772_PDObject[AP33772_PDO_LIST_SIZE];
	this->srcPDOCount = 0;
	this->desiredPDO = desiredPDO;
	this->temp.value = 0;
}

AP33772Driver::~AP33772Driver()
{
	delete[] this->buffer;
	delete[] this->srcPDOList;
}

HAL_StatusTypeDef AP33772Driver::Init()
{
	// Read PDOs from the chip.
	if (this->ReadSourcePDOs() != HAL_OK) {
		return HAL_ERROR;
	}
	this->FindNearestPDO();
	return HAL_OK;
}

void AP33772Driver::HandleInterrupt()
{
	if (this->GetStatus() != HAL_OK) {
		return;
	}

	// handle the different types of status flags...
}

HAL_StatusTypeDef AP33772Driver::GetStatus()
{
	if (this->ReadRegister(AP33772_STATUS, 1) != HAL_OK) {
		return HAL_ERROR;
	}
	if ((this->buffer[0] & 1) == 1) {
		this->status.value = this->buffer[0];
	}
	return HAL_OK;
}

HAL_StatusTypeDef AP33772Driver::GetVoltage()
{
	if (this->ReadRegister(AP33772_VOLTAGE, 1) != HAL_OK) {
		return HAL_ERROR;
	}
	this->measured.voltage = this->buffer[0] * 0.08;
	return HAL_OK;
}

HAL_StatusTypeDef AP33772Driver::GetCurrent()
{
	if (this->ReadRegister(AP33772_CURRENT, 1) != HAL_OK) {
		return HAL_ERROR;
	}
	this->measured.current = this->buffer[0] * 0.024;
	return HAL_OK;
}

HAL_StatusTypeDef AP33772Driver::GetTemp()
{
	if (this->ReadRegister(AP33772_TEMP, 1) != HAL_OK) {
		return HAL_ERROR;
	}
	this->measured.temp = this->buffer[0];
	return HAL_OK;
}

AP33772_MeasuredValues* AP33772Driver::GetMeasuredValues()
{
	return &this->measured;
}

HAL_StatusTypeDef AP33772Driver::ReadSourcePDOs()
{
	if (this->ReadRegister(AP33772_PDONUM, 1) != HAL_OK) {
		return HAL_ERROR;
	}
	this->srcPDOCount = this->buffer[0];

	if (this->srcPDOCount > 7) return HAL_ERROR;

	if (this->ReadRegister(AP33772_SRCPDO, 28) != HAL_OK) {
		return HAL_ERROR;
	}

	for (int i = 0; i < this->srcPDOCount; ++i) {
		this->srcPDOList[i] = this->ParsePDO(this->buffer + (4 * i));
	}

	return HAL_OK;
}

HAL_StatusTypeDef AP33772Driver::ReadRegister(AP33772_Register cmd, uint8_t len)
{
	this->buffer[0] = cmd;
	if (HAL_I2C_Master_Transmit(this->hi2c, this->address, this->buffer, 1, 100) != HAL_OK) {
		return HAL_ERROR;
	}
	if (HAL_I2C_Master_Receive(this->hi2c, this->address, this->buffer, len, 100) != HAL_OK) {
		return HAL_ERROR;
	}
	return HAL_OK;
}

AP33772_PDObject AP33772Driver::ParsePDO(uint8_t *buf)
{
	AP33772_PDObject output = {};
	this->temp.data[0] = buf[0];
	this->temp.data[1] = buf[1];
	this->temp.data[2] = buf[2];
	this->temp.data[3] = buf[3];

	output.pdoType = (AP33772_PDOType)(this->temp.data[3] >> 6);

	if (output.pdoType == AP33772_FIXED_PDO) {
		output.current = (this->temp.value & 0x3FF) * 0.01;
		output.minVoltage = ((this->temp.value >> 10) & 0x3FF) * 0.05;
		output.maxVoltage = 0;
	} else {
		output.current = (this->temp.value & 0x3F) * 0.05;
		output.minVoltage = ((this->temp.value >> 8) & 0x3F) * 0.1;
		output.maxVoltage = ((this->temp.value >> 17) & 0x3F) * 0.1;
	}
	return output;
}

void AP33772Driver::FindNearestPDO()
{
	if (this->desiredPDO.voltage == 0) {
		this->foundPDO = false;
		this->voltageMismatch = false;
		return;
	}
	int i = 0;

	// Search for a matching fixed voltage PDO
	for (i = 0; i < this->srcPDOCount; ++i) {
		if (this->srcPDOList[i].pdoType == AP33772_FIXED_PDO) {
			if (this->srcPDOList[i].current >= this->desiredPDO.maxCurrent) {
				if (this->srcPDOList[i].minVoltage == this->desiredPDO.voltage) {
					this->selectedPDOIndex = i;
					this->foundPDO = true;
					this->voltageMismatch = false;
					return;
				}
			}
		}
	}

	// If a fixed PDO cant be found, find a adjustable PDO that has the required voltage range.
	for (i = 0; i < this->srcPDOCount; ++i) {
		if (this->srcPDOList[i].pdoType == AP33772_ADJ_PDO) {
			if (this->srcPDOList[i].current >= this->desiredPDO.maxCurrent) {
				if (this->srcPDOList[i].minVoltage <= this->desiredPDO.voltage && this->srcPDOList[i].maxVoltage >= this->desiredPDO.voltage) {
					this->selectedPDOIndex = i;
					this->foundPDO = true;
					this->voltageMismatch = false;
					return;
				}
			}
		}
	}

	// If there are no matching or programmable PDOs, find the closest fixed match.
	int8_t closestPDOindex = -1;
	float closestDiff = 0;
	float temp = 0;
	for (i = 0; i < this->srcPDOCount; ++i) {
		if (this->desiredPDO.current > this->srcPDOList[i].current) {
			continue;
		}
		temp = std::abs(this->desiredPDO.voltage - this->srcPDOList[i].minVoltage);
		if (temp < closestDiff) {
			closestDiff = temp;
			closestPDOindex = i;
		}
	}
	if (closestPDOindex > -1) {
		this->selectedPDOIndex = closestPDOindex;
		this->foundPDO = true;
		this->voltageMismatch = true;
		return;
	}

	this->foundPDO = false;
}

HAL_StatusTypeDef AP33772Driver::SendRequestedPDO()
{
	if (this->foundPDO) {
		this->temp.value = ((this->selectedPDOIndex + 1) << 28);
		if (this->srcPDOList[this->selectedPDOIndex].pdoType == AP33772_FIXED_PDO) {

			this->temp.value |= ((uint32_t)(this->desiredPDO.maxCurrent / 0.01) & 0x3FF);
			this->temp.value |= (((uint32_t)(this->desiredPDO.current / 0.01) & 0x3FF) << 10);
		} else {
			this->temp.value |= ((uint32_t)(this->desiredPDO.current / 0.05) & 0x3F);
			this->temp.value |= (((uint32_t)(this->desiredPDO.voltage / 0.02) & 0x3FF) << 9);
		}

		this->buffer[0] = AP33772_RDO;
		this->buffer[1] = this->temp.data[0];
		this->buffer[2] = this->temp.data[1];
		this->buffer[3] = this->temp.data[2];
		this->buffer[4] = this->temp.data[3];

		if (HAL_I2C_Master_Transmit(this->hi2c, this->address, this->buffer, 5, 200) != HAL_OK) {
			return HAL_ERROR;
		}

		return HAL_OK;
	}
	return HAL_ERROR;
}

HAL_StatusTypeDef AP33772Driver::SetInterrupts(AP33772_Status enabledInts)
{
	this->enabledInterrupts.status = enabledInts;
	this->buffer[0] = AP33772_MASK;
	this->buffer[1] = this->enabledInterrupts.value;
	if (HAL_I2C_Master_Transmit(this->hi2c, this->address, this->buffer, 2, 100) != HAL_OK) {
		return HAL_ERROR;
	}
	return HAL_OK;
}

HAL_StatusTypeDef AP33772Driver::SetOverTemp(uint8_t temp)
{
	this->buffer[0] = AP33772_OTPTHR;
	this->buffer[1] = temp;
	return HAL_I2C_Master_Transmit(this->hi2c, this->address, this->buffer, 2, 200);
}

void AP33772Driver::GetTempRes()
{
	// Used this for testing. This is not an actual implementation.
	this->ReadRegister(AP33772_TR25, 2);

//	uint16_t temp = this->buffer[0] | (this->buffer[1] << 8);
}
