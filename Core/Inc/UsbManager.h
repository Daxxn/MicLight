/*
 * UsbManager.h
 *
 *  Created on: Dec 17, 2024
 *      Author: Daxxn
 */

#ifndef INC_USBMANAGER_H_
#define INC_USBMANAGER_H_

#include "main.h"
#include "usb_device.h"

#define USB_RX_BUFFER_SIZE 64
#define USB_TX_BUFFER_SIZE 32
#define USB_CALLBACK_LIST_SIZE 255

typedef enum {
	USB_CMD_SET_BRIGHT,
	USB_CMD_SET_
} UsbCommand;

typedef void (*UsbDataReceiveCallback) (uint8_t* buffer, uint32_t len);

/*
 *
 */
class UsbManager
{
public:
	UsbManager() { };
	UsbManager(_USBD_HandleTypeDef *usbHandle);
	virtual ~UsbManager();

	void AddCallback(UsbCommand id, UsbDataReceiveCallback callback);

	void ReceivedCallback(uint8_t *buffer, uint32_t *len);

	void Update();
private:
	_USBD_HandleTypeDef *Handle;
	uint8_t  RxBuffer[USB_RX_BUFFER_SIZE];
	uint8_t  TxBuffer[USB_TX_BUFFER_SIZE];
	uint32_t RxLen  = 0;
	uint32_t TxLen  = 0;
	bool     RxFlag = false;
	bool     TxFlag = false;

	UsbDataReceiveCallback Callbacks[USB_CALLBACK_LIST_SIZE];

	void ParseCommand();
};

#endif /* INC_USBMANAGER_H_ */
