/*
 * StateManager.cpp
 *
 *  Created on: Dec 27, 2024
 *      Author: Daxxn
 */

#include <StateManager.h>

StateManager::StateManager(LightingManager *lighting, UsbManager *usb)
{
	Lighting = lighting;
	Usb = usb;
}

StateManager::~StateManager()
{
	Lighting = nullptr;
	Usb = nullptr;
}

HAL_StatusTypeDef StateManager::Init()
{
	if (Lighting->Init() != HAL_OK) {
		State = STATE_ERROR;
		return HAL_ERROR;
	}
	State = STATE_INIT;
	return HAL_OK;
}

void StateManager::Update()
{
	switch (State) {
		case STATE_PRE_INIT:

			break;
		case STATE_INIT:

			break;
		case STATE_LIGHT_OFF:

			break;
		case STATE_LIGHT_ON:

			break;
		case STATE_LIGHT_MAX:

			break;
		case STATE_SLEEP:

			break;
		case STATE_OVER_HEAT:

			break;
		case STATE_OVER_CURRRENT:

			break;
		case STATE_ERROR:

			break;
		default:
			break;
	}
}
