/********************************************************************
 * Include Files
 ********************************************************************/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#include <rt_misc.h>

#include "eat_modem.h"
#include "eat_interface.h"
#include "eat_uart.h"


#include "Definedata.h"
#include "UartTask.h"


#define 	DEBUG		1

static s8 LdebugPrintBuff[250];
static u8 DebugFileEnableInf = 0;
static u8 InforFileEnableInf = 0;
static u8 uer_debug_working = 0;
static u8 uer_infor_working = 0;
//static u8 Ldebug_sending = 0;
void user_debug_unable(void){
	DebugFileEnableInf = 0;
	InforFileEnableInf = 0;
}
void user_debug_enable(void)
{
	DebugFileEnableInf = 1;
	InforFileEnableInf = 1;
	uer_debug_working = 0;
	uer_infor_working = 0;
}

u32 user_debug(const char *format, ...)
{
 va_list args;
 
 if(0x55 == uer_debug_working)return 0;
 uer_debug_working = 0x55;
 //eat_trace("user_debug:%d",DebugFileEnableInf);
 if(1 == DebugFileEnableInf)
 {
 	memset(LdebugPrintBuff, 0 , 250);
  	va_start (args, format);

   	vsprintf (&LdebugPrintBuff[2], format, args);
   	va_end (args);
   	//eat_uart_write(EAT_UART_1, &LdebugPrintBuff[2], strlen(&LdebugPrintBuff[0x02]));//add by lileieat_trace(&LdebugPrintBuff[2]);
   	LdebugPrintBuff[0] = 0x09;
   	LdebugPrintBuff[1] = 0x00;
   	//eat_uart_write(EAT_UART_1, &LdebugPrintBuff[2], strlen(&LdebugPrintBuff[0x02]) + 1);    //add by lilei-2016-04-06
   	//obd_write(&LdebugPrintBuff[2], strlen(&LdebugPrintBuff[0x02]) + 1);     //add by lilei-2016-04-06
   	obd_write(LdebugPrintBuff, strlen(&LdebugPrintBuff[0x02]) + 3);     
 }
 uer_debug_working = 0x00;
 return 0;
}

u32 user_infor(const char *format, ...)
{
 va_list args;
 
 //eat_trace("user_debug:%d",DebugFileEnableInf);
 if(0x55 == uer_infor_working)return 0;
 uer_infor_working = 0x55;
 if(1 == InforFileEnableInf)
 {
 	memset(LdebugPrintBuff, 0 , 250);
  	va_start (args, format);

   	vsprintf (LdebugPrintBuff, format, args);
   	va_end (args);
    	eat_trace(LdebugPrintBuff);
   	vsprintf (&LdebugPrintBuff[2], format, args);
   	va_end (args);
    	//eat_uart_write(EAT_UART_1, &LdebugPrintBuff[2], strlen(&LdebugPrintBuff[0x02]));//add by lilei-2016-05-06;
   	LdebugPrintBuff[0] = 0x09;
   	LdebugPrintBuff[1] = 0x01;
   	//eat_uart_write(EAT_UART_1, &LdebugPrintBuff[2], strlen(&LdebugPrintBuff[0x02]) + 1);    //add by lilei-2016-04-06
   	//obd_write(&LdebugPrintBuff[2], strlen(&LdebugPrintBuff[0x02]) + 1);     //add by lilei-2016-04-06
   	obd_write(LdebugPrintBuff, strlen(&LdebugPrintBuff[0x02]) + 2);
  
 }
 uer_infor_working = 0x00;
 return 0;
}

/*******************************************************************************
 * 功能描述：转换1个数字字符为数字是否为'0~9, a~f, A~F',并
 				   判断该字符是否为数字字符
 * 输入参数：isrc数据
 * 输出参数：无
 * 返回参数：转换后的数据
 *           			   -1: 出错
 * 外部调用：无
 * 注意事项：无
 *******************************************************************************/
s16 AsciiToHex( u8 isrc)
{
    s8 itemp = 0;

    itemp = isrc;
	if (('0' <= itemp)&&('9'>=itemp))
    {
        itemp -= '0';
    }
    else if (('a' <= itemp) && ('f' >= itemp))
    {
        itemp -= 'a';
        itemp += 0x0a;
    }
    else if (('A' <= itemp) && ('F' >= itemp))
    {
        itemp -= 'A';
        itemp += 0x0a;
    }
    else
    {
        itemp = (s8)-1;
    }
    return itemp;
}


/*******************************************************************************
 * 功能描述：将2个字符串合并成一个字节的数据，并将结果
 * 				   存放在目的地址中
 * 输入参数：psrc 字符串指针，如果输入的字符串非16进制的
 *                             数据话就返回-1，否则为0
 * 输出参数：转换后的数据
 * 返回参数：-1: 错误
 *                             0: 正确
 * 外部调用：无
 * 注意事项：无
 ******************************************************************************
 */
s16 AsciiToByte(const u8 * psrc, u8 * pdst )
{
    s8 itemph = 0;
    s8 itempl = 0;
 
    itemph = AsciiToHex(*psrc);

    if ((s8)-1 == itemph )
    {
        return -1;
    }
    itempl = AsciiToHex(*(psrc+1));
    if ((s8)-1 == itempl )
    {
        return -1;
    }

    itemph <<=4;
    itemph |= itempl;

    *pdst = itemph;
	return 0;
}

/*************************************************************************
 * 功能描述：将HEX数据转为ASCII 待转换的数据必须为0-f 否则不转换原样输出
             hex为非压缩BDC码，转换后的数据为小写字符
 * 输入参数：hexdata= hex数据
             datalen = hex字节数
 * 输出参数：NULL
 * 返回参数：转换后的数据 使用一个全局数组作为输出数据
 *           NULL失败
 * 外部调用：无
 * 注意事项：无
*/
#define HEX_STR_BUF_MAX 254
static u8 Hex2Str_Buf[HEX_STR_BUF_MAX];
u8 *Hex2Str(u8 *hexdata, u16 datalen){
	u16 dataindex;
	if(NULL == hexdata || datalen >= HEX_STR_BUF_MAX)return NULL;
	for(dataindex = 0; dataindex < datalen; dataindex ++){
		if(*(hexdata + dataindex) >= 0 && *(hexdata + dataindex) <= 9)Hex2Str_Buf[dataindex] = 0x30 + *(hexdata + dataindex);
		else if(*(hexdata + dataindex) >= 0x0a && *(hexdata + dataindex) <= 0x0f)Hex2Str_Buf[dataindex] = 'a' + (*(hexdata + dataindex) - 0x0a);
		else Hex2Str_Buf[dataindex] = *(hexdata + dataindex);
	}
	Hex2Str_Buf[dataindex] = 0;
	return Hex2Str_Buf;
}
u8 *u32Str(u32 data){
	u8 datat,i;
	
	memset(Hex2Str_Buf, 0, 254);
	for(i = 0; i < 8; i ++){
	    datat = (data >> (28 - i * 4)) & 0x0f;
	    if(datat <= 9)Hex2Str_Buf[i] = datat + '0';
	    else Hex2Str_Buf[i] = (datat - 0x0a) + 'A';
  }
  return Hex2Str_Buf;
}
u8 *u16Str(u16 data){
	u8 datat,i;
	
	memset(Hex2Str_Buf, 0, 254);
	for(i = 0; i < 4; i ++){
	    datat = (data >> (12 - i * 4)) & 0x0f;
	    if(datat <= 9)Hex2Str_Buf[i] = datat + '0';
	    else Hex2Str_Buf[i] = (datat - 0x0a) + 'A';
  }
  return Hex2Str_Buf;
}

u8 *u8Str(u8 data){
	u8 datat,i;
	
	memset(Hex2Str_Buf, 0, 254);
	for(i = 0; i < 2; i ++){
	    datat = (data >> (4 - i * 4)) & 0x0f;
	    if(datat <= 9)Hex2Str_Buf[i] = datat + '0';
	    else Hex2Str_Buf[i] = (datat - 0x0a) + 'A';
  }
  return Hex2Str_Buf;
}
/********************************************************************

 ********************************************************************/
u8 * get_gps(u8 * tail , u8 * str)
{
	u8  i;
	u8* pt;

	if (*str == '\0')	return tail;
	while (tail != (PcTool_Rx_buf+EAT_UART_RX_BUF_LEN_MAX)) {
		pt = tail;
		for (i = 0; *pt == *(str+i); i++) {
			if (*(str+i+1) == '\0')	return tail;
			pt++;
		}
		tail++;
	}
	return NULL;
}

/********************************************************************
//             1                2   3                  4    5                   6   7         8         9
//$GPRMC , 053933.00 , A , 2232.89301 , N , 11356.21651 , E , 6.731 , 56.44 , 101011,,,A*52
//$GPRMC,084951.00,V,,,,,,,101011,,,N*7C
 ********************************************************************/
static void GpsRmcData(u8 * prt , u16 len){
	u8 *DataHand;
	u8 *Datatail ;
	u8 cnt = 0;
	char i=1;
	u8  checksum = 0;
	u8  gpsck = 0;
	u8 GpsBuf[200];
	
	DataHand = get_gps(prt , "$GPRMC,");
	Datatail = get_gps(prt , "\r\n");

	if(Datatail  < DataHand){
		user_debug("i:GPRMC rcv fail !!!!\r\n");
		return;
	}
	cnt =Datatail  - DataHand ;
	if(cnt > len) {
		user_debug("i:GPRMC rcv fail !!!!\r\n");
		return;
	}
	
	memcpy(&GpsBuf, DataHand, cnt);
	GpsBuf[cnt] = '\0';
	
#if DEBUG
	user_debug("i:%s\r\n" , GpsBuf);
#endif

	while( GpsBuf[i] != '*' ){
		checksum = checksum ^ GpsBuf[i];
		i++;
		if(i>cnt) break;
	}
	i++;
	AsciiToByte(&GpsBuf[i], &gpsck);	
	if (gpsck != checksum){
		user_debug("i:GPRMC rcv fail !!!!\r\n");
		return;
	}

}

/********************************************************************
 //$GPGSA,A,3,20,32,04,28,11,07,17,08,,,,,2.27,1.00,2.03*01
 //$GPGSA,A,1,,,,,,,,,,,,,99.99,99.99,99.99*30
 ********************************************************************/
static void GpsGsaData(u8 * prt , u16 len){

	u8 GpsBuf[200];
	u8 *DataHand;
	u8 *Datatail ;
	u8 cnt= 0;
	char i=1;
	char n = 0;
	u8  checksum = 0;
	u8  gpsck = 0;
	
	memset(GpsBuf, 0, 200);
	DataHand = get_gps(prt, "$GPGSA");
	Datatail = get_gps(prt , "\r\n");

	if(Datatail  < DataHand){
		user_debug("i:GPGSA rcv fail !!!!\r\n");
		return;
	}
	cnt =Datatail  - DataHand ;
	if(cnt > len) {
		user_debug("i:GPGSA rcv fail !!!!\r\n");
		return;
	}

	memcpy(&GpsBuf, DataHand, cnt);
	GpsBuf[cnt] = '\0';

#if DEBUG
	user_debug("i:%s\r\n" , GpsBuf);
#endif

	while( GpsBuf[i] != '*' ){
		checksum = checksum ^ GpsBuf[i];
		i++;
		if(i>cnt) break;
	}
	i++;
	AsciiToByte(&GpsBuf[i], &gpsck);	
	if (gpsck != checksum){
		user_debug("i:GPGSA rcv fail !!!!\r\n");
		return;
	}
}

/********************************************************************
 * GPS数据 接收
 ********************************************************************/
static void GpsDataFun(u8 * prt , u16 len){

	if(len>50){
		if(get_gps(prt , "$GPRMC,")){
			GpsRmcData(prt , len);
		}
		if(get_gps(prt, "$GPGSA,")){
			GpsGsaData(prt , len);
		}		
	}
	else{
		user_debug("i:gps rcv fail !!!!\r\n");
	}
}

/********************************************************************
 * PCTOOL 串口设置
 ********************************************************************/
static void PctoolDataFun(u8 * prt , u16 len){


}


void app_test_single_block(u8 *prt , u16 len)
{

	eat_bool ret = EAT_FALSE;

	u32 base_addr=0;
	u32 app_space = 0;
	u32 addr = 0;
	u8 buf[10];

	base_addr = eat_get_app_base_addr();
	app_space = eat_get_app_space() ;
	addr = base_addr+app_space-0x1000;
	
	user_debug("i:Flash addr=%x, len=%x ,addr=%x ", base_addr, app_space,addr);
	ret = eat_flash_erase((char *)addr, len);

	if( ret ){
		ret = eat_flash_write((char *)addr, prt, len);
		if(ret){
			user_debug("i:eat_flash_write ok");
		}
		else{
			user_debug("i:eat_flash_write fail");
		}
	}
	else{
		return ;
	}
	memset(buf , 0x00 , 10);
	memcpy(buf, (char *)addr, 9);
	user_debug("i:get str:%s",buf);

	memset(buf , 0x00 , 10);
	memcpy(buf, (char *)(addr+10) , 9);
	user_debug("i:get str:%s",buf);

}

/********************************************************************
 * 串口接收任务
 ********************************************************************/
void uart_rx_proc(const EatEvent_st* event)
{
	u16 len;
	u8 *debug;
	EatUart_enum uart = event->data.uart.uart;

	if(uart == EAT_UART_1){
		len = eat_uart_read(uart, PcTool_Rx_buf, EAT_UART_RX_BUF_LEN_MAX);
		if(len != 0){
			//PctoolDataFun((u8 *)&PcTool_Rx_buf,len);
			//app_test_single_block(PcTool_Rx_buf,len);
			//GpsDataFun(PcTool_Rx_buf,len);
			debug = Hex2Str(PcTool_Rx_buf,len);
			if(debug != NULL)user_debug("i:OBD[%d]:%s",strlen(debug),debug);
		}
	}
	if(uart == EAT_UART_2){
		len = eat_uart_read(uart, GPS_Rx_buf, EAT_UART_RX_BUF_LEN_MAX);
		if(len != 0){
			//GpsDataFun(GPS_Rx_buf,len);
			debug = Hex2Str(GPS_Rx_buf,len);
			if(debug != NULL)user_debug("i:GPS[%d]:%s",strlen(debug),debug);
		}
	}
}

/*打印HEX数据
*/
static u8 Lhexdebug[250];
static u8 debug_hex_working = 0;
void debug_hex(u8 * fmt, u8 *data, u16 datalen){
	u16 dataindex,datamax;
	u8 debugt[4],u8data;
	
	return;//2015/12/1 10:23 fangcuisong
	if(0x55 == debug_hex_working)return;
	debug_hex_working = 0x55;
	if(NULL == data || 0 == datalen){
		debug_hex_working = 0x00;
		return;
	}
	if(fmt != NULL)user_infor(fmt);
	if(datalen > 250){
		debug_hex_working = 0x00;
		return;
	}
	memset(Lhexdebug, 0, 256);
	if(datalen < 10)datamax = datalen;
	else datamax = 10;
	for(dataindex = 0; dataindex < datamax; dataindex ++){
		sprintf(debugt, " %02x", *(data + dataindex));
		strcat(Lhexdebug, debugt);
	}
	user_infor(Lhexdebug);
	debug_hex_working = 0x00;
}




