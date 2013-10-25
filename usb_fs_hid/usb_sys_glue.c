#include "stm32f4xx_conf.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "semphr.h"

void usb_connect_notify(void)
{
	GPIO_SetBits(GPIOD, GPIO_Pin_15);
}

void usb_disconnect_notify(void)
{
	GPIO_ResetBits(GPIOD, GPIO_Pin_15);
}
