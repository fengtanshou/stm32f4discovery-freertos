#include "stm32f4xx_conf.h"

void EXTI0_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line0) != RESET)
	{
		/* Toggle LED4 */
		GPIO_ToggleBits(GPIOD, GPIO_Pin_15);

		/* Clear the EXTI line 0 pending bit */
		EXTI_ClearITPendingBit(EXTI_Line0);
	}
}
