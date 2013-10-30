#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "stm32f4xx_conf.h"
#include "stm32f4_discovery_lis302dl.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "usbd_cdc_core.h"
#include "usbd_cdc_vcp.h"
#include "usbd_desc.h"
#include "usbd_usr.h"
#include "usb_conf.h"

#include "tim2.h"

__ALIGN_BEGIN USB_OTG_CORE_HANDLE  USB_OTG_dev __ALIGN_END;

GPIO_InitTypeDef gpio_init;
NVIC_InitTypeDef nvic_init;
EXTI_InitTypeDef exti_init;

LIS302DL_InitTypeDef  acc_init;
LIS302DL_FilterConfigTypeDef acc_filter_init;

xSemaphoreHandle blue_sig;
xSemaphoreHandle usb_rx_sig;

static void hw_init(void)
{
	/* GPIOD Periph clock enable */

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	/* Configure PD12, PD13, PD14 and PD15 in output pushpull mode */

	gpio_init.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	gpio_init.GPIO_Mode = GPIO_Mode_OUT;
	gpio_init.GPIO_OType = GPIO_OType_PP;
	gpio_init.GPIO_Speed = GPIO_Speed_100MHz;
	gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;

	GPIO_Init(GPIOD, &gpio_init);

	/* Enable GPIOA clock */

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	/* Enable SYSCFG clock */

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	/* Configure PA0 pin as input floating */

	gpio_init.GPIO_Pin = GPIO_Pin_0;
	gpio_init.GPIO_Mode = GPIO_Mode_IN;
	gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;

	GPIO_Init(GPIOA, &gpio_init);

	/* Connect EXTI Line0 to PA0 pin */

	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);

	/* Configure EXTI Line0 */

	exti_init.EXTI_Line = EXTI_Line0;
	exti_init.EXTI_Mode = EXTI_Mode_Interrupt;
	exti_init.EXTI_Trigger = EXTI_Trigger_Rising;
	exti_init.EXTI_LineCmd = ENABLE;

	EXTI_Init(&exti_init);

	/* Enable and set EXTI Line0 Interrupt to the lowest priority */

	nvic_init.NVIC_IRQChannel = EXTI0_IRQn;
	nvic_init.NVIC_IRQChannelPreemptionPriority = 0x01;
	nvic_init.NVIC_IRQChannelSubPriority = 0x01;
	nvic_init.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic_init);

	/* Enable TIM2 */

	timer_tim2_init();

	/* Enable accelerometer */

	acc_init.Power_Mode = LIS302DL_LOWPOWERMODE_ACTIVE;
	acc_init.Output_DataRate = LIS302DL_DATARATE_100;
	acc_init.Axes_Enable = LIS302DL_XYZ_ENABLE;
	acc_init.Full_Scale = LIS302DL_FULLSCALE_2_3;
	acc_init.Self_Test = LIS302DL_SELFTEST_NORMAL;
	LIS302DL_Init(&acc_init);

	/* Required delay for accelerometer: turn-on time = 3/(output data rate) = 3/100 = 30ms */

	delay(30);

	/* Enable MEMS high pass filter */

	acc_filter_init.HighPassFilter_Data_Selection = LIS302DL_FILTEREDDATASELECTION_OUTPUTREGISTER;
	acc_filter_init.HighPassFilter_CutOff_Frequency = LIS302DL_HIGHPASSFILTER_LEVEL_1;
	acc_filter_init.HighPassFilter_Interrupt = LIS302DL_HIGHPASSFILTERINTERRUPT_1_2;
	LIS302DL_FilterConfig(&acc_filter_init);

	/* Enable USB device */

	USBD_Init(&USB_OTG_dev, USB_OTG_FS_CORE_ID, &USR_desc, &USBD_CDC_cb, &USR_cb);

	/*
		http://www.freertos.org/RTOS-Cortex-M3-M4.html
		Preempt priority and subpriority:

			If you are using an STM32 with the STM32 driver library then ensure all the
			priority bits are assigned to be preempt priority bits by calling
			NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 ); before the RTOS is started.
	*/

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	/* DON'T PUT ANYTHING HERE AFTER NVIC PRIO GROUP ARE PROPERLY CONFIGURED FOR FREERTOS */
}

static void LedFlash(void *Parameters)
{
	portTickType LastWake;

	LastWake = xTaskGetTickCount();

	GPIO_SetBits(GPIOD, GPIO_Pin_12);

	while(1) {
		GPIO_ToggleBits(GPIOD, GPIO_Pin_12);
		vTaskDelayUntil(&LastWake, 1000);
	}
}

static void VcpEchoTask(void *Parameters)
{
	uint8_t ch;

	GPIO_ResetBits(GPIOD, GPIO_Pin_13);

	while(1) {
		xSemaphoreTake(usb_rx_sig, portMAX_DELAY);
		while (VCP_get_char(&ch)) {
			if (ch == '\r') {
				VCP_put_char('\n');
				VCP_put_char('\r');
			} else {
				VCP_put_char(ch);
			}
		}

		GPIO_ToggleBits(GPIOD, GPIO_Pin_13);
	}
}

static void BlueLedControl(void *Parameters)
{
	uint8_t buffer[6];

	GPIO_SetBits(GPIOD, GPIO_Pin_14);

	while(1) {
		xSemaphoreTake(blue_sig, portMAX_DELAY);
		GPIO_ToggleBits(GPIOD, GPIO_Pin_14);

		LIS302DL_Read(buffer, LIS302DL_OUT_X_ADDR, 6);
		printf("[0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x]\n\r",
			buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
	}
}

int main()
{
	/* init OS structures */

	/* FIXME: don't send over usb until it is not connected */
	vSemaphoreCreateBinary(usb_rx_sig);
	xSemaphoreTake(usb_rx_sig, portMAX_DELAY);

	vSemaphoreCreateBinary(blue_sig);
	xSemaphoreTake(blue_sig, portMAX_DELAY);

	/* init hw */

	hw_init();
	blink(5, GPIOD, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);

	/* setup tasks */

	xTaskCreate(LedFlash, (signed char *) "led", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL);

	xTaskCreate(BlueLedControl, (signed char *) "button", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, NULL);

	xTaskCreate(VcpEchoTask, (signed char *) "usb", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);

	vTaskStartScheduler();

	/* Just sit and flash all the LED quickly if we fail */

	critical_error_blink();
}
