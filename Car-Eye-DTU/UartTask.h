/* car-eye车辆管理平台 
 * car-eye车辆管理公共平台   www.car-eye.cn
 * car-eye开源网址:  https://github.com/Car-eye-admin
 * Copyright car-eye 车辆管理平台  2018
 */



#if !defined (__UART_TASK__)
#define __UART_TASK__

#include "eat_interface.h"

extern void uart_rx_proc(const EatEvent_st* event);
void debug_hex(u8 * fmt, u8 *data, u16 datalen);

u8 *u32Str(u32 data);
u8 *u16Str(u16 data);
u8 *u8Str(u8 data);

void user_debug_enable(void);
void user_debug_unable(void);
u32 user_debug(const char *format, ...);
u32 user_infor(const char *format, ...);
#endif




