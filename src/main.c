#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "stm32f4xx_conf.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "stm32f4_discovery_lis302dl.h"
#include "usbd_cdc_vcp.h"
#include "tim2.h"

xSemaphoreHandle blue_sig;
xSemaphoreHandle usb_rx_sig;

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

	/* 'ok' blink */

	blink(5, GPIOD, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);

	/* setup tasks */

	xTaskCreate(LedFlash, (signed char *) "led", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL);
	xTaskCreate(BlueLedControl, (signed char *) "button", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, NULL);
	xTaskCreate(VcpEchoTask, (signed char *) "usb", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);

	/* 'ok' blink */

	blink(5, GPIOD, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);

	/* kick-off scheduler */

	vTaskStartScheduler();

	/* Just sit and flash all the LED quickly if we fail */

	critical_error_blink();
}
