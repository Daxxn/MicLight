/*
 * UnionTypes.h
 *
 *  Created on: Nov 22, 2024
 *      Author: Daxxn
 */

#ifndef INC_UNIONTYPES_H_
#define INC_UNIONTYPES_H_

#include <stdint.h>

typedef union {
	uint16_t integer;
	uint8_t bytes[2];
} UInt16Union;

typedef union {
	uint32_t integer;
	uint8_t bytes[4];
} UInt32Union;

typedef union {
	uint64_t integer;
	uint8_t bytes[8];
} UInt64Union;

typedef union {
	int16_t integer;
	uint8_t bytes[2];
} Int16Union;

typedef union {
	int32_t integer;
	uint8_t bytes[4];
} Int32Union;

typedef union {
	int64_t integer;
	uint8_t bytes[8];
} Int64Union;

typedef union {
	float number;
	uint8_t bytes[4];
} FloatUnion;

typedef union {
	double number;
	uint8_t bytes[8];
} DoubleUnion;

#endif /* INC_UNIONTYPES_H_ */
