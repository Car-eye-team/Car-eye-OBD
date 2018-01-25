
#ifndef     _INCLUDESH_
#define     _INCLUDESH_

typedef enum {FALSE = 0, TRUE = !FALSE} bool;

//#define 		HUABAO_VER_EN

//#define 	DEBUG_TRACE_EN		//调试信息打印使能//

#define    TXH_UART_APP
//#define    BLUETOOTH_UART_APP
#endif

#include    <stdio.h>
#include    <string.h>
#include    <ctype.h>
#include    <stdlib.h>
#include    <setjmp.h>
#include    <stdarg.h>
#include	<math.h>

#include    "stm32f10x.h"


#include    "main.h"

#include    "bsp.h"
#include    "tools.h"
#include	"Driver_can.h"
#include 	"Obdii.h"
#include    "Pidarith.h"
#include    "OilArith.h"
#include    "protocol.h"
#include    "bspobdii.h"



























