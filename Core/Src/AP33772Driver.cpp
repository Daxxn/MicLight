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
	this->srcPDOCount = 0;
	this->temp.integer = 0;
	for (uint8_t i = 0; i < AP33772_BUFFER_SIZE; ++i) {
		this->buffer[i] = 0;
	}
	for (uint8_t i = 0; i < AP33772_TX_BUFFER_SIZE; ++i) {
		this->txBuffer[i] = 0;
	}
}

AP33772Driver::AP33772Driver(I2C_HandleTypeDef *hi2c, AP33772_PDRequestObject *desiredPDO)
{
	this->hi2c = hi2c;
	this->address = AP33772_ADDRESS << 1;
	this->srcPDOCount = 0;
	this->desiredPDO = desiredPDO;
	this->temp.integer = 0;
	for (uint8_t i = 0; i < AP33772_BUFFER_SIZE; ++i) {
		this->buffer[i] = 0;
	}
	for (uint8_t i = 0; i < AP33772_TX_BUFFER_SIZE; ++i) {
		this->txBuffer[i] = 0;
	}
}

AP33772Driver::~AP33772Driver()
{
	this->hi2c = nullptr;
}

HAL_StatusTypeDef AP33772Driver::Init()
{
	// Read PDOs from the chip.
	if (this->ReadSourcePDOs() != HAL_OK) {
		return HAL_ERROR;
	}
	this->FindNearestPDO();
	if (!this->foundPDO) {
		return HAL_ERROR;
	}
	return HAL_OK;
}

void AP33772Driver::StatusInterrupt()
{
	if (this->GetStatus() != HAL_OK) {
		return;
	}

//	if (this->status.newPDO) {
//		foundPDO = false;
//		newPDOFlag = true;
//	}
	// handle the different types of status flags...
}

void AP33772Driver::FaultIndInterrupt()
{
//	uint32_t currTick = HAL_GetTick();
//	if (currTick )
}

HAL_StatusTypeDef AP33772Driver::GetStatus()
{
	if (this->ReadRegister(AP33772_CMD_STATUS, 1) != HAL_OK) {
		return HAL_ERROR;
	}
	this->status.ready       = (this->buffer[0] & 0b00000001);
	if (!this->status.ready) {
		return HAL_ERROR;
	}
	this->status.success     = (this->buffer[0] & 0b00000010) != 0;
	this->status.newPDO      = (this->buffer[0] & 0b00000100) != 0;
	this->status.overVolt    = (this->buffer[0] & 0b00010000) != 0;
	this->status.overCurrent = (this->buffer[0] & 0b00100000) != 0;
	this->status.overTemp    = (this->buffer[0] & 0b01000000) != 0;
	this->status.derating    = (this->buffer[0] & 0b10000000) != 0;
	return HAL_OK;
}

HAL_StatusTypeDef AP33772Driver::GetVoltage()
{
	if (this->ReadRegister(AP33772_CMD_VOLTAGE, 1) != HAL_OK) {
		return HAL_ERROR;
	}
	this->measured.voltage = this->buffer[0] * 0.08;
	return HAL_OK;
}

HAL_StatusTypeDef AP33772Driver::GetCurrent()
{
	if (this->ReadRegister(AP33772_CMD_CURRENT, 1) != HAL_OK) {
		return HAL_ERROR;
	}
	this->measured.current = this->buffer[0] * 0.024;
	return HAL_OK;
}

HAL_StatusTypeDef AP33772Driver::GetTemp()
{
	if (this->ReadRegister(AP33772_CMD_TEMP, 1) != HAL_OK) {
		return HAL_ERROR;
	}
	this->measured.temp = (float)this->buffer[0];
	return HAL_OK;
}

HAL_StatusTypeDef AP33772Driver::GetMeasurements()
{
	if (GetVoltage() != HAL_OK) {
		return HAL_ERROR;
	}
	if (GetCurrent() != HAL_OK) {
		return HAL_ERROR;
	}
	return HAL_OK;
}

HAL_StatusTypeDef AP33772Driver::SetThermistorValues()
{
	UInt16Union tempUnion;
	tempUnion.integer = this->thermValues.t25;
	this->txBuffer[0] = tempUnion.bytes[0];
	this->txBuffer[1] = tempUnion.bytes[1];
	tempUnion.integer = this->thermValues.t50;
	this->txBuffer[2] = tempUnion.bytes[0];
	this->txBuffer[3] = tempUnion.bytes[1];
	tempUnion.integer = this->thermValues.t75;
	this->txBuffer[4] = tempUnion.bytes[0];
	this->txBuffer[5] = tempUnion.bytes[1];
	tempUnion.integer = this->thermValues.t100;
	this->txBuffer[6] = tempUnion.bytes[0];
	this->txBuffer[7] = tempUnion.bytes[1];
	return this->WriteRegister(AP33772_CMD_TR25, 8);
}

AP33772_PDObject* AP33772Driver::GetSelectedPDO()
{
	if (selectedPDOIndex < srcPDOCount) {
		return &srcPDOList[selectedPDOIndex];
	}
	return nullptr;
}

AP33772_MeasuredValues* AP33772Driver::GetMeasuredValuesPtr()
{
	return &this->measured;
}

HAL_StatusTypeDef AP33772Driver::ReadSourcePDOs()
{
	if (this->ReadRegister(AP33772_CMD_PDONUM, 1) != HAL_OK) {
		return HAL_ERROR;
	}
	this->srcPDOCount = this->buffer[0];

	if (this->srcPDOCount > 7) return HAL_ERROR;

	if (this->ReadRegister(AP33772_CMD_SRCPDO, 28) != HAL_OK) {
		return HAL_ERROR;
	}

	for (uint8_t i = 0; i < this->srcPDOCount; ++i) {
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

HAL_StatusTypeDef AP33772Driver::WriteRegister(AP33772_Register cmd, uint8_t len)
{
	this->buffer[0] = cmd;
	for (uint8_t i = 0; i < len; ++i) {
		this->buffer[i + 1] = this->txBuffer[i];
	}
	return HAL_I2C_Master_Transmit(hi2c, address, buffer, len + 1, 200);
}

AP33772_PDObject AP33772Driver::ParsePDO(uint8_t *buf)
{
	AP33772_PDObject output = {};
	this->temp.bytes[0] = buf[0];
	this->temp.bytes[1] = buf[1];
	this->temp.bytes[2] = buf[2];
	this->temp.bytes[3] = buf[3];

	output.pdoType = (AP33772_PDOType)(this->temp.bytes[3] >> 6);

	if (output.pdoType == AP33772_FIXED_PDO) {
		output.current = (this->temp.integer & 0x3FF) * 0.01;
		output.minVoltage = ((this->temp.integer >> 10) & 0x3FF) * 0.05;
		output.maxVoltage = 0;
	} else {
		output.current = (this->temp.integer & 0x3F) * 0.05;
		output.minVoltage = ((this->temp.integer >> 8) & 0x3F) * 0.1;
		output.maxVoltage = ((this->temp.integer >> 17) & 0x3F) * 0.1;
	}
	return output;
}

//void AP33772Driver::FindNearestPDO_OLD()
//{
//	if (this->srcPDOCount == 0 || this->desiredPDO.voltage == 0) {
//		this->foundPDO = false;
//		this->voltageMismatch = false;
//		return;
//	}
//
//	int i = 0;
//
//	// Search for a matching fixed voltage PDO
//	for (i = 0; i < this->srcPDOCount; ++i) {
//		if (this->srcPDOList[i].pdoType == AP33772_FIXED_PDO) {
//			if (this->srcPDOList[i].current >= this->desiredPDO.maxCurrent) {
//				if (this->srcPDOList[i].minVoltage == this->desiredPDO.voltage) {
//					this->selectedPDOIndex = i;
//					this->foundPDO = true;
//					this->voltageMismatch = false;
//					return;
//				}
//			}
//		}
//	}
//
//	// If a fixed PDO cant be found, find a adjustable PDO that has the required voltage range.
//	for (i = 0; i < this->srcPDOCount; ++i) {
//		if (this->srcPDOList[i].pdoType == AP33772_ADJ_PDO) {
//			if (this->srcPDOList[i].current >= this->desiredPDO.maxCurrent) {
//				if (this->srcPDOList[i].minVoltage <= this->desiredPDO.voltage && this->srcPDOList[i].maxVoltage >= this->desiredPDO.voltage) {
//					this->selectedPDOIndex = i;
//					this->foundPDO = true;
//					this->voltageMismatch = false;
//					return;
//				}
//			}
//		}
//	}
//
//	// If there are no matching or programmable PDOs, find the closest fixed match.
//	int8_t closestPDOindex = -1;
//	float closestDiff = 0;
//	float temp = 0;
//	for (i = 0; i < this->srcPDOCount; ++i) {
//		if (this->desiredPDO.current > this->srcPDOList[i].current) {
//			continue;
//		}
//		temp = std::abs(this->desiredPDO.voltage - this->srcPDOList[i].minVoltage);
//		if (temp < closestDiff) {
//			closestDiff = temp;
//			closestPDOindex = i;
//		}
//	}
//	if (closestPDOindex > -1) {
//		this->selectedPDOIndex = closestPDOindex;
//		this->foundPDO = true;
//		this->voltageMismatch = true;
//		return;
//	}
//
//	this->foundPDO = false;
//}

void AP33772Driver::FindNearestPDO()
{
	if (this->srcPDOCount == 0 || this->desiredPDO->voltage == 0) {
		this->foundPDO = false;
		this->voltageMismatch = false;
		return;
	}

	uint8_t i = 0;

	// Check fixed PDOs for an exact match:
	for (i = 0; i < this->srcPDOCount; ++i) {
		if (this->srcPDOList[i].pdoType == AP33772_FIXED_PDO) {
			if (this->srcPDOList[i].minVoltage == this->desiredPDO->voltage) {
				if (this->srcPDOList[i].current >= this->desiredPDO->current) {
					this->selectedPDOIndex = i;
					this->foundPDO = true;
					this->voltageMismatch = false;
					return;
				}
			}
		}
	}

	// Otherwise, check PDOs for an adjustable PDO:
	for (i = 0; i < this->srcPDOCount; ++i) {
		if (this->srcPDOList[i].pdoType == AP33772_ADJ_PDO) {
			if (this->srcPDOList[i].minVoltage <= this->desiredPDO->voltage && this->srcPDOList[i].maxVoltage >= this->desiredPDO->voltage) {
				if (this->srcPDOList[i].current >= this->desiredPDO->current) {
					this->selectedPDOIndex = i;
					this->foundPDO = true;
					this->voltageMismatch = false;
					return;
				}
			}
		}
	}

	// Finally, check for a fixed PDO that can be derated to work:

	for (i = 0; i < this->srcPDOCount; ++i) {
		if (this->srcPDOList[i].pdoType == AP33772_FIXED_PDO) {
			if (this->srcPDOList[i].minVoltage >= this->desiredPDO->voltage) {
				this->selectedPDOIndex = i;
				this->desiredPDO->current = this->srcPDOList[i].current;
				this->foundPDO = true;
				this->voltageMismatch = false;
				return;
			}
		}
	}

	this->foundPDO = false;
	this->voltageMismatch = false;
}

HAL_StatusTypeDef AP33772Driver::SendRequestedPDO()
{
	if (this->foundPDO) {
		this->temp.integer = ((this->selectedPDOIndex + 1) << 28);
		if (this->srcPDOList[this->selectedPDOIndex].pdoType == AP33772_FIXED_PDO) {
			if (this->desiredPDO->maxCurrent > this->desiredPDO->current) {
				this->temp.integer |= ((uint32_t)(this->desiredPDO->current / 0.01) & 0x3FF);
				this->temp.integer |= (((uint32_t)(this->desiredPDO->current / 0.01) & 0x3FF) << 10);
			} else {
				this->temp.integer |= ((uint32_t)(this->desiredPDO->maxCurrent / 0.01) & 0x3FF);
				this->temp.integer |= (((uint32_t)(this->desiredPDO->current / 0.01) & 0x3FF) << 10);
			}
		} else {
			this->temp.integer |= ((uint32_t)(this->desiredPDO->current / 0.05) & 0x3F);
			this->temp.integer |= (((uint32_t)(this->desiredPDO->voltage / 0.02) & 0x3FF) << 9);
		}

//		this->buffer[0] = AP33772_RDO;
//		this->buffer[1] = this->temp.bytes[0];
//		this->buffer[2] = this->temp.bytes[1];
//		this->buffer[3] = this->temp.bytes[2];
//		this->buffer[4] = this->temp.bytes[3];
//		if (HAL_I2C_Master_Transmit(this->hi2c, this->address, this->buffer, 5, 200) != HAL_OK) {
//			return HAL_ERROR;
//		}

		for (uint8_t i = 0; i < 4; ++i) {
			this->txBuffer[i] = this->temp.bytes[i];
		}
		this->WriteRegister(AP33772_CMD_RDO, 4);

		return HAL_OK;
	}
	return HAL_ERROR;
}

HAL_StatusTypeDef AP33772Driver::SetInterrupts(AP33772_Status enabledInts)
{
	this->enabledInterrupts = enabledInts;
//	this->buffer[0] = AP33772_MASK;
//	this->buffer[1] = this->enabledInterrupts.value;
//	if (HAL_I2C_Master_Transmit(this->hi2c, this->address, this->buffer, 2, 100) != HAL_OK) {
//		return HAL_ERROR;
//	}
//	return HAL_OK;
	this->txBuffer[1]  = this->enabledInterrupts.ready;
	this->txBuffer[1] |= this->enabledInterrupts.success     << 1;
	this->txBuffer[1] |= this->enabledInterrupts.newPDO      << 2;
	this->txBuffer[1] |= this->enabledInterrupts.overVolt    << 3;
	this->txBuffer[1] |= this->enabledInterrupts.overCurrent << 4;
	this->txBuffer[1] |= this->enabledInterrupts.overTemp    << 5;
	this->txBuffer[1] |= this->enabledInterrupts.derating    << 6;
	return this->WriteRegister(AP33772_CMD_MASK, 1);
}

HAL_StatusTypeDef AP33772Driver::SetOverTemp(uint8_t temp)
{
//	this->buffer[0] = AP33772_OTPTHR;
//	this->buffer[1] = temp;
//	return HAL_I2C_Master_Transmit(this->hi2c, this->address, this->buffer, 2, 200);
	this->txBuffer[0] = temp;
	return this->WriteRegister(AP33772_CMD_OTPTHR, 1);
}

AP33772_ThermValues* AP33772Driver::GetThermValues()
{
	UInt16Union tempU16;
	if (this->ReadRegister(AP33772_CMD_TR25, 8) != HAL_OK) {
		return &thermValues;
	}

	tempU16.bytes[0] = this->buffer[0];
	tempU16.bytes[1] = this->buffer[1];
	thermValues.t25 = tempU16.integer;
	tempU16.bytes[0] = this->buffer[2];
	tempU16.bytes[1] = this->buffer[3];
	thermValues.t50 = tempU16.integer;
	tempU16.bytes[0] = this->buffer[4];
	tempU16.bytes[1] = this->buffer[5];
	thermValues.t75 = tempU16.integer;
	tempU16.bytes[0] = this->buffer[6];
	tempU16.bytes[1] = this->buffer[7];
	thermValues.t100 = tempU16.integer;

	return &thermValues;
}
