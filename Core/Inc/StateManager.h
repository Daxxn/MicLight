/*
 * StateManager.h
 *
 *  Created on: Dec 27, 2024
 *      Author: Daxxn
 */

#ifndef INC_STATEMANAGER_H_
#define INC_STATEMANAGER_H_

#include "main.h"
#include "LightingManager.h"
#include "UsbManager.h"

typedef enum {
	STATE_PRE_INIT,
	STATE_INIT,
	STATE_LIGHT_OFF,
	STATE_LIGHT_ON,
	STATE_LIGHT_MAX,
	STATE_SLEEP,
	STATE_OVER_HEAT,
	STATE_OVER_CURRRENT,
	STATE_ERROR,
} LightingState;

/*
 *
 */
class StateManager
{
public:
	StateManager() { };
	StateManager(LightingManager *lighting, UsbManager *usb);
	virtual ~StateManager();

	HAL_StatusTypeDef Init();

	void Update();
private:
	LightingManager *Lighting;
	UsbManager *Usb;
	LightingState State = STATE_PRE_INIT;
};

#endif /* INC_STATEMANAGER_H_ */
