#include "stm32f4xx_conf.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

extern xQueueHandle xQueue;

unsigned char c = 'E';

void EXTI0_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line0) != RESET)
	{
		xQueueSendToFrontFromISR(xQueue, &c, NULL);

		/* Clear the EXTI line 0 pending bit */
		EXTI_ClearITPendingBit(EXTI_Line0);
	}
}
