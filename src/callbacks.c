#include "stm32f4xx_conf.h"

uint32_t LIS302DL_TIMEOUT_UserCallback(void)
{
	critical_error_blink();
}
