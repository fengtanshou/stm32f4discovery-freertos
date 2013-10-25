#include "stm32f4xx_conf.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "semphr.h"

extern xSemaphoreHandle usb_rx_sig;

void usb_rx_notify(void)
{
	xSemaphoreGive(usb_rx_sig);
}

void usb_connect_notify(void)
{
	GPIO_SetBits(GPIOD, GPIO_Pin_15);
}

void usb_disconnect_notify(void)
{
	GPIO_ResetBits(GPIOD, GPIO_Pin_15);
}
