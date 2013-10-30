#include "stm32f4xx_conf.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "usbd_usr.h"

extern USB_OTG_CORE_HANDLE  USB_OTG_dev;
extern xSemaphoreHandle blue_sig;

void EXTI0_IRQHandler(void)
{
	signed portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	if (EXTI_GetITStatus(EXTI_Line0) != RESET)
	{
		xSemaphoreGiveFromISR(blue_sig, &xHigherPriorityTaskWoken);
		portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);

		/* Clear the EXTI line 0 pending bit */
		EXTI_ClearITPendingBit(EXTI_Line0);
	}
}

void OTG_FS_IRQHandler(void)
{
	USBD_OTG_ISR_Handler(&USB_OTG_dev);
}

void TIM2_IRQHandler(void)
{
	timer_tim2_irq();
}
