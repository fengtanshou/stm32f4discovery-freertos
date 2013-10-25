/**
 ******************************************************************************
 * @file    usbd_cdc_vcp.c
 * @author  MCD Application Team
 * @version V1.0.0
 * @date    22-July-2011
 * @brief   Generic media access Layer.
 ******************************************************************************
 * @attention
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
 ******************************************************************************
 */

#include "usbd_cdc_vcp.h"
#include "usb_sys_glue.h"

LINE_CODING linecoding = {
	115200,	/* baud rate*/
	0x00,	/* stop bits-1*/
	0x00,	/* parity - none*/
	0x08	/* nb. of bits 8*/
};

/*
   Write CDC received data in this buffer.
   These data will be sent over USB IN endpoint in the CDC core functions.
 */

extern uint8_t APP_Rx_Buffer[];

/*
   Increment this pointer or roll it back to start address when writing
   received data in the buffer APP_Rx_Buffer.
 */

extern uint32_t APP_Rx_ptr_in;

static uint16_t VCP_Init(void);
static uint16_t VCP_DeInit(void);
static uint16_t VCP_Ctrl(uint32_t Cmd, uint8_t* Buf, uint32_t Len);
static uint16_t VCP_DataTx(uint8_t* Buf, uint32_t Len);
static uint16_t VCP_DataRx(uint8_t* Buf, uint32_t Len);

CDC_IF_Prop_TypeDef VCP_fops = {
	VCP_Init,
	VCP_DeInit,
	VCP_Ctrl,
	VCP_DataTx,
	VCP_DataRx
};

static uint16_t VCP_Init(void)
{
	return USBD_OK;
}

static uint16_t VCP_DeInit(void)
{
	return USBD_OK;
}

static uint16_t VCP_Ctrl(uint32_t Cmd, uint8_t* Buf, uint32_t Len)
{
	switch (Cmd) {
		case SEND_ENCAPSULATED_COMMAND:
			break;

		case GET_ENCAPSULATED_RESPONSE:
			break;

		case SET_COMM_FEATURE:
			break;

		case GET_COMM_FEATURE:
			break;

		case CLEAR_COMM_FEATURE:
			break;

		case SET_LINE_CODING:
			break;

		case GET_LINE_CODING:
			Buf[0] = (uint8_t) (linecoding.bitrate);
			Buf[1] = (uint8_t) (linecoding.bitrate >> 8);
			Buf[2] = (uint8_t) (linecoding.bitrate >> 16);
			Buf[3] = (uint8_t) (linecoding.bitrate >> 24);
			Buf[4] = linecoding.format;
			Buf[5] = linecoding.paritytype;
			Buf[6] = linecoding.datatype;
			break;

		case SET_CONTROL_LINE_STATE:
			break;

		case SEND_BREAK:
			break;

		default:
			break;
	}

	return USBD_OK;
}

void VCP_put_char(uint8_t buf)
{
	VCP_DataTx(&buf, 1);
}

void VCP_send_str(uint8_t* buf)
{
	uint32_t i = 0;

	while (*(buf + i)) {
		i++;
	}

	VCP_DataTx(buf, i);
}

void VCP_send_buffer(uint8_t* buf, int len)
{
	VCP_DataTx(buf, len);
}

static uint16_t VCP_DataTx(uint8_t* Buf, uint32_t Len)
{
	uint32_t i = 0;

	while (i < Len) {
		APP_Rx_Buffer[APP_Rx_ptr_in] = *(Buf + i);
		APP_Rx_ptr_in++;
		i++;

		/* To avoid buffer overflow */
		if (APP_Rx_ptr_in == APP_RX_DATA_SIZE) {
			APP_Rx_ptr_in = 0;
		}
	}

	return USBD_OK;
}

/**
 * @brief  VCP_DataRx
 *         Data received over USB OUT endpoint are sent over CDC interface
 *         through this function.
 *
 *         @note
 *         This function will block any OUT packet reception on USB endpoint
 *         until exiting this function. If you exit this function before transfer
 *         is complete on CDC interface (ie. using DMA controller) it will result
 *         in receiving more data while previous ones are still not sent.
 *
 * @param  Buf: Buffer of data to be received
 * @param  Len: Number of data received (in bytes)
 * @retval Result of the opeartion: USBD_OK if all operations are OK else VCP_FAIL
 */

#define APP_TX_BUF_SIZE 128

uint8_t APP_Tx_Buffer[APP_TX_BUF_SIZE];
uint32_t APP_tx_ptr_head;
uint32_t APP_tx_ptr_tail;

static uint16_t VCP_DataRx(uint8_t* Buf, uint32_t Len)
{
	uint16_t ret = USBD_OK;
	uint32_t i;

	for (i = 0; i < Len; i++) {
		APP_Tx_Buffer[APP_tx_ptr_head] = *(Buf + i);
		APP_tx_ptr_head++;
		if (APP_tx_ptr_head == APP_TX_BUF_SIZE)
			APP_tx_ptr_head = 0;

		if (APP_tx_ptr_head == APP_tx_ptr_tail) {
			ret = USBD_FAIL;
			break;
		}
	}

	usb_rx_notify();

	return ret;
}

int VCP_get_char(uint8_t *buf)
{
	if (APP_tx_ptr_head == APP_tx_ptr_tail)
		return 0;

	*buf = APP_Tx_Buffer[APP_tx_ptr_tail];
	APP_tx_ptr_tail++;
	if (APP_tx_ptr_tail == APP_TX_BUF_SIZE)
		APP_tx_ptr_tail = 0;

	return 1;
}

int VCP_get_string(uint8_t *buf)
{
	if (APP_tx_ptr_head == APP_tx_ptr_tail)
		return 0;

	while (!APP_Tx_Buffer[APP_tx_ptr_tail]
			|| APP_Tx_Buffer[APP_tx_ptr_tail] == '\n'
			|| APP_Tx_Buffer[APP_tx_ptr_tail] == '\r') {

		APP_tx_ptr_tail++;

		if (APP_tx_ptr_tail == APP_TX_BUF_SIZE)
			APP_tx_ptr_tail = 0;

		if (APP_tx_ptr_head == APP_tx_ptr_tail)
			return 0;
	}

	int i = 0;
	do {
		*(buf + i) = APP_Tx_Buffer[i + APP_tx_ptr_tail];
		i++;

		if ((APP_tx_ptr_tail + i) == APP_TX_BUF_SIZE)
			i = -APP_tx_ptr_tail;
		if (APP_tx_ptr_head == (APP_tx_ptr_tail + i))
			return 0;

	} while (APP_Tx_Buffer[APP_tx_ptr_tail + i]
			&& APP_Tx_Buffer[APP_tx_ptr_tail + i] != '\n'
			&& APP_Tx_Buffer[APP_tx_ptr_tail + i] != '\r');

	*(buf + i) = 0;
	APP_tx_ptr_tail += i;
	if (APP_tx_ptr_tail >= APP_TX_BUF_SIZE)
		APP_tx_ptr_tail -= APP_TX_BUF_SIZE;
	return i;
}
