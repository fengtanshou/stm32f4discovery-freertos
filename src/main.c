#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "stm32f4xx_conf.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

static void delay(volatile uint32_t count)
{
	while(count) {
		count--;
	}
}

GPIO_InitTypeDef  gpio_init;

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
}

static void LedFlash1(void *Parameters)
{
	portTickType LastWake;

	GPIO_SetBits(GPIOD, GPIO_Pin_12);

	LastWake = xTaskGetTickCount();

	while(1) {
		GPIO_ToggleBits(GPIOD, GPIO_Pin_12);
		vTaskDelayUntil(&LastWake, 1000);
	}
}

static void LedFlash2(void *Parameters)
{
	portTickType LastWake;

	GPIO_SetBits(GPIOD, GPIO_Pin_14);

	LastWake = xTaskGetTickCount();

	while(1) {
		GPIO_ToggleBits(GPIOD, GPIO_Pin_14);
		vTaskDelayUntil(&LastWake, 2000);
	}
}

static led_check(int leds)
{
	volatile int i = 5;

	while (i--) {
		GPIO_SetBits(GPIOD, leds);
		delay(0x3FFFFF);
		GPIO_ResetBits(GPIOD, leds);
		delay(0x3FFFFF);
	}
}

int main()
{

	hw_init();

	led_check(GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);

	xTaskCreate(LedFlash1, (signed char *) "a", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL);

	led_check(GPIO_Pin_12 | GPIO_Pin_14);

	xTaskCreate(LedFlash2, (signed char *) "b", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, NULL);

	led_check(GPIO_Pin_13 | GPIO_Pin_15);

	vTaskStartScheduler();

	/* Just sit and flash all the LED quickly if we fail */

	while( 1 ) {
		GPIO_ResetBits(GPIOD, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
		delay(0x3FFFFF);
		GPIO_SetBits(GPIOD, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
		delay(0x3FFFFF);
	}
}
