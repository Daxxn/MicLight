/*
 * AP33772Types.h
 *
 *  Created on: May 21, 2024
 *      Author: Daxxn
 */

#ifndef INC_AP33772TYPES_H_
#define INC_AP33772TYPES_H_

#define AP33772_ADDRESS 0x51
#define AP33772_BUFFER_SIZE 36
#define AP33772_PDO_LIST_SIZE 7
#define AP33772_INIT_RETRY_COUNT 10

typedef enum : uint8_t {
	AP33772_SRCPDO  = 0x00, // Get source PDO capabilities
	AP33772_PDONUM  = 0x1C, // Get valid source PDO number (Status flag for the current state of the power negotiation?)
	AP33772_STATUS  = 0x1D, // Get the status flags of the chip.
	AP33772_MASK    = 0x1E, // Write the interrupt mask for the I2C_INT pin.
	AP33772_VOLTAGE = 0x20, // Get the voltage of VBUS.
	AP33772_CURRENT = 0x21, // Get the current through VBUS.
	AP33772_TEMP    = 0x22, // Get the temp measured by the thermistor.
	AP33772_OCPTHR  = 0x23, // Get/Set the over current protection threshold.
	AP33772_OTPTHR  = 0x24, // Get/Set the ocer-temp threshold.
	AP33772_DRTHR   = 0x25, // Get/Set the derating temperature threshold.
	AP33772_TR25    = 0x28, // Get/Set the thermal resistance @ 25C.
	AP33772_TR50    = 0x2A, // Get/Set the thermal resistance @ 50C.
	AP33772_TR75    = 0x2C, // Get/Set the thermal resistance @ 75C.
	AP33772_TR100   = 0x2E, // Get/Set the thermal resistance @ 100C.
	AP33772_RDO     = 0x30, // Set the RDO (Request Data Object) for the chip to get from the supply.
							//   Write all 0s to this register to perform a hard reset of the chip.)
	AP33772_VENID   = 0x34, // Get the vendor ID from the chip.
 // AP33772_PRODID  = 0x36,    Get the product ID from the chip. (Reserved for future use.)
 // AP33772_RESERVED = 0x38,   Reserved for future use.
} AP33772_Register;

typedef enum : uint8_t {
	AP33772_FIXED_PDO = 0b00,
	AP33772_ADJ_PDO   = 0b11,
	AP33772_ANY_PDO   = 0x01,
} AP33772_PDOType;

typedef struct {
	bool ready;
	bool success;
	bool newPDO;
	bool reserved;
	bool ovpStatus;
	bool ocpStatus;
	bool otpStatus;
	bool deratingStatus;
} AP33772_Status;

typedef union {
	AP33772_Status status;
	uint8_t value;
} AP33772_StatusUnion;

typedef union {
	uint8_t data[4];
	uint32_t value;
} AP33772_PDOUnion;

typedef struct {
	float current;
	float minVoltage; // Acts as the Voltage value for Fixed PDOs.
	float maxVoltage;
	AP33772_PDOType pdoType;
} AP33772_PDObject;

typedef struct {
	float maxCurrent;
	float current;
	float voltage;
	AP33772_PDOType pdoType;
} AP33772_PDRequestObject;

static const AP33772_PDRequestObject defaultPDO = {
		.maxCurrent = 0.5,
		.current = 0.25,
		.voltage = 5,
		.pdoType = AP33772_FIXED_PDO,
};

typedef struct {
	float voltage;
	float current;
	uint8_t temp;
} AP33772_MeasuredValues;

#endif /* INC_AP33772TYPES_H_ */
