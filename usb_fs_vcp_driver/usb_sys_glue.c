#include "stm32f4xx_conf.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "semphr.h"

extern xSemaphoreHandle usb_rx_sig;

void usb_rx_notify(void)
{
	xSemaphoreGive(usb_rx_sig);
}
