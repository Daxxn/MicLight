/*
 * UsbManager.cpp
 *
 *  Created on: Dec 17, 2024
 *      Author: Daxxn
 */

#include <UsbManager.h>

UsbManager::UsbManager(_USBD_HandleTypeDef *usbHandle)
{
	Handle = usbHandle;
	for (uint8_t i = 0; i < USB_CALLBACK_LIST_SIZE; ++i) {
		Callbacks[i] = nullptr;
	}
}

UsbManager::~UsbManager()
{
	Handle = nullptr;
}

void UsbManager::AddCallback(UsbCommand cmd, UsbDataReceiveCallback callback)
{
	Callbacks[cmd] = callback;
}

void UsbManager::ReceivedCallback(uint8_t *buffer, uint32_t *len)
{
	assert_param(*len <= USB_RX_BUFFER_SIZE);
	for (uint32_t i = 0; i < *len; ++i) {
		RxBuffer[i] = buffer[i];
	}
	RxLen = *len;
	RxFlag = true;
}

void UsbManager::ParseCommand()
{
	UsbCommand currentCommand = (UsbCommand)RxBuffer[0];
	if (Callbacks[currentCommand] != nullptr) {
		Callbacks[currentCommand](RxBuffer + 1, RxLen);
	}
}

void UsbManager::Update()
{
	if (RxFlag) {
		ParseCommand();
	}

	if (TxFlag) {
		UsbTransmitData(TxBuffer, TxLen);
		TxFlag = false;
	}
}
