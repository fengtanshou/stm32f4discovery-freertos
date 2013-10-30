#include <stdint.h>

#include "stm32f4xx_conf.h"

#include "stm32f4_discovery_audio_codec.h"
#include "stm32f4_discovery_lis302dl.h"

#include "usbd_usr.h"
#include "usbd_ioreq.h"

/* MEMS accelerometer callback */

uint32_t LIS302DL_TIMEOUT_UserCallback(void)
{
	critical_error_blink();
}

/* Audio callbacks */

void EVAL_AUDIO_TransferComplete_CallBack(uint32_t buffer, uint32_t size)
{
	EVAL_AUDIO_Stop(CODEC_PDWN_HW);
}

uint32_t Codec_TIMEOUT_UserCallback(void)
{

}

uint16_t EVAL_AUDIO_GetSampleCallBack(void)
{

}

/* USB device callbacks */

USBD_Usr_cb_TypeDef USR_cb =
{
	USBD_USR_Init,
	USBD_USR_DeviceReset,
	USBD_USR_DeviceConfigured,
	USBD_USR_DeviceSuspended,
	USBD_USR_DeviceResumed,
	USBD_USR_DeviceConnected,
	USBD_USR_DeviceDisconnected,
};

void USBD_USR_Init(void)
{

}

void USBD_USR_DeviceReset(uint8_t speed )
{
	switch (speed)
	{
		case USB_OTG_SPEED_HIGH:
			break;

		case USB_OTG_SPEED_FULL:
			break;

		default:
			break;
	}
}

void USBD_USR_DeviceConfigured (void)
{
	GPIO_SetBits(GPIOD, GPIO_Pin_15);
}

void USBD_USR_DeviceConnected (void)
{
	GPIO_SetBits(GPIOD, GPIO_Pin_15);
}

void USBD_USR_DeviceDisconnected (void)
{
	GPIO_ResetBits(GPIOD, GPIO_Pin_15);
}

void USBD_USR_DeviceSuspended(void)
{
	GPIO_ResetBits(GPIOD, GPIO_Pin_15);
}

void USBD_USR_DeviceResumed(void)
{
	GPIO_SetBits(GPIOD, GPIO_Pin_15);
}
