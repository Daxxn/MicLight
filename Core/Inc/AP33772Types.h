/*
 * AP33772Types.h
 *
 *  Created on: May 21, 2024
 *      Author: Daxxn
 */

#ifndef INC_AP33772TYPES_H_
#define INC_AP33772TYPES_H_

#include "UnionTypes.h"

#define AP33772_ADDRESS 0x51
#define AP33772_BUFFER_SIZE 36
#define AP33772_TX_BUFFER_SIZE 10
#define AP33772_PDO_LIST_SIZE 7
#define AP33772_INIT_RETRY_COUNT 10

typedef enum : uint8_t {
	AP33772_CMD_SRCPDO  = 0x00, // Get source PDO capabilities
	AP33772_CMD_PDONUM  = 0x1C, // Get valid source PDO number (Status flag for the current state of the power negotiation?)
	AP33772_CMD_STATUS  = 0x1D, // Get the status flags of the chip.
	AP33772_CMD_MASK    = 0x1E, // Write the interrupt mask for the I2C_INT pin.
	AP33772_CMD_VOLTAGE = 0x20, // Get the voltage of VBUS.
	AP33772_CMD_CURRENT = 0x21, // Get the current through VBUS.
	AP33772_CMD_TEMP    = 0x22, // Get the temp measured by the thermistor.
	AP33772_CMD_OCPTHR  = 0x23, // Get/Set the over current protection threshold.
	AP33772_CMD_OTPTHR  = 0x24, // Get/Set the ocer-temp threshold.
	AP33772_CMD_DRTHR   = 0x25, // Get/Set the derating temperature threshold.
	AP33772_CMD_TR25    = 0x28, // Get/Set the thermal resistance @ 25C.
	AP33772_CMD_TR50    = 0x2A, // Get/Set the thermal resistance @ 50C.
	AP33772_CMD_TR75    = 0x2C, // Get/Set the thermal resistance @ 75C.
	AP33772_CMD_TR100   = 0x2E, // Get/Set the thermal resistance @ 100C.
	AP33772_CMD_RDO     = 0x30, // Set the RDO (Request Data Object) for the chip to get from the supply.
							    //   Write all 0s to this register to perform a hard reset of the chip.)
	AP33772_CMD_VENID   = 0x34, // Get the vendor ID from the chip.
 // AP33772_PRODID  = 0x36,    Get the product ID from the chip. (Reserved for future use.)
 // AP33772_RESERVED = 0x38,   Reserved for future use.
} AP33772_Register;

typedef enum : uint8_t {
	AP33772_FIXED_PDO = 0b00,
	AP33772_ADJ_PDO   = 0b11,
	AP33772_ANY_PDO   = 0x01,
} AP33772_PDOType;

typedef struct {
	bool ready       = false;
	bool success     = false;
	bool newPDO      = false;
	bool reserved    = false;
	bool overVolt    = false;
	bool overCurrent = false;
	bool overTemp    = false;
	bool derating    = false;
} AP33772_Status;

const AP33772_Status interruptMask = {
		ready: true,
		success: true,
		newPDO: false,
		overVolt: true,
		overCurrent: true,
		overTemp: true,
		derating: true,
};

//typedef union {
//	AP33772_Status status;
//	uint8_t value = 0;
//} AP33772_StatusUnion;

//typedef union {
//	uint8_t data[4];
//	uint32_t value;
//} AP33772_PDOUnion;

typedef struct {
	uint16_t t25  = 10000;
	uint16_t t50  = 4161;
	uint16_t t75  = 1928;
	uint16_t t100 = 974;
} AP33772_ThermValues;

const AP33772_ThermValues defaultThermValues = {
		t25: 10000,
		t50: 4165,
		t75: 1929,
		t100: 956
};

typedef struct {
	float current           = 0; // Current in Amps from the supply.
	float minVoltage        = 0; // Minimum supply voltage. If PDO type is fixed; Acts as the Voltage output for Fixed PDOs.
	float maxVoltage        = 0; // Maximum supply voltage. If POD type is fixed; NOT used.
	AP33772_PDOType pdoType = AP33772_FIXED_PDO; // The type of the source power supply.
} AP33772_PDObject;

typedef struct {
	float maxCurrent        = 0;
	float current           = 0;
	float voltage           = 0;
	AP33772_PDOType pdoType = AP33772_FIXED_PDO;
} AP33772_PDRequestObject;

typedef struct {
	float maxCurrent = 0;
	float minCurrent = 0;
	float minVoltage = 0;
	float maxVoltage = 0;
} DesiredPDObject;

static const AP33772_PDRequestObject defaultPDO = {
		.maxCurrent = 0.5,
		.current    = 0.25,
		.voltage    = 5,
		.pdoType    = AP33772_FIXED_PDO,
};

typedef struct {
	float voltage = 0;
	float current = 0;
	float temp    = 0;
} AP33772_MeasuredValues;

#endif /* INC_AP33772TYPES_H_ */
