#include "stm32f4xx_conf.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "usbd_hid_core.h"
#include "usbd_desc.h"
#include "usbd_usr.h"
#include "usb_conf.h"

extern USB_OTG_CORE_HANDLE  USB_OTG_dev;
extern xSemaphoreHandle blue_sig;

void EXTI0_IRQHandler(void)
{
	signed portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	if(EXTI_GetITStatus(EXTI_Line0) != RESET)
	{
		xSemaphoreGiveFromISR(blue_sig, &xHigherPriorityTaskWoken);
		portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);

		/* Clear the EXTI line 0 pending bit */
		EXTI_ClearITPendingBit(EXTI_Line0);
	}
}

void OTG_FS_WKUP_IRQHandler(void)
{
	if(USB_OTG_dev.cfg.low_power)
	{
		 *(uint32_t *)(0xE000ED10) &= 0xFFFFFFF9 ;
		SystemInit();
		USB_OTG_UngateClock(&USB_OTG_dev);
	}

	EXTI_ClearITPendingBit(EXTI_Line18);
}

void OTG_FS_IRQHandler(void)
{
	USBD_OTG_ISR_Handler(&USB_OTG_dev);
}
