/**
  ******************************************************************************
  * @file    usbd_cdc_vcp.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    22-July-2011
  * @brief   Header for usbd_cdc_vcp.c file.
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

#ifndef __USBD_CDC_VCP_H
#define __USBD_CDC_VCP_H

#include "usbd_cdc_core.h"
#include "usbd_conf.h"

typedef struct {
	uint32_t bitrate;
	uint8_t  format;
	uint8_t  paritytype;
	uint8_t  datatype;
} LINE_CODING;

void VCP_put_char(uint8_t buf);
void VCP_send_str(uint8_t* buf);
int VCP_get_char(uint8_t *buf);
int VCP_get_string(uint8_t *buf);
void VCP_send_buffer(uint8_t* buf, int len);

#define DEFAULT_CONFIG                  0
#define OTHER_CONFIG                    1

#endif /* __USBD_CDC_VCP_H */
