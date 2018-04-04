/********************************************************************
 *                Copyright Simcom(shanghai)co. Ltd.                   *
 *---------------------------------------------------------------------
 * FileName      :   main.c
 * version       :   0.10
 * Description   :   
 * Authors       :   maobin
 * Notes         :
 *---------------------------------------------------------------------
 *
 *    HISTORY OF CHANGES
 *---------------------------------------------------------------------
 *0.10  2012-09-24, maobin, create originally.
 *
 *--------------------------------------------------------------------
 * File Description
 * AT+CEAT=parma1,param2
 * param1 param2 
 *   
 *--------------------------------------------------------------------
 ********************************************************************/
/********************************************************************
 * Include Files
 ********************************************************************/
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <rt_misc.h>

#include "eat_modem.h"
#include "eat_interface.h"
#include "eat_uart.h"
#include "definedata.h"
#include "UartTask.h"
#include "AppUser1.h"
#include "OBDapi.h"
#include "AppUser2.h"
#include "gps.h"
#include "db.h"
#include "BigMem.h"
#include "ATapi.h"



/********************************************************************
 * Macros
 ********************************************************************/
#define EAT_CORE_APP_INDEPEND
//线程启动标志
u32 Ltaskstartflag = 0;//任务启动标志
/********************************************************************
 * Types
 ********************************************************************/
typedef void (*app_user_func)(void*);
typedef void (*event_handler_func)(const EatEvent_st *event);

/*
*系统睡眠
*1、M2M接收到OBD的睡眠请求后进入设置睡眠标志
*2、main发送睡眠指令到需要的自任务 子任务进入睡眠并设置睡眠标志
*3、main进入睡眠
*OBD通过设置M2M的中断线电平唤醒M2M
*1、MAIN醒来后发送唤醒命令到各子任务
**********************************************************/
static u8 system_status = 0;/*0 = 正常
                              1 = 进入睡眠
                              2 = 唤醒
                            */
static u8 system_status_flag = 0;
static u8 system_subtask_status = 0;
void system_status_set(u8 flag){
	user_debug("i:system_status_set=%d",flag);
	system_status = flag;
	if(1 == system_status)system_status_flag = 0x55;
}
void system_subtask_status_set(u8 flag){
	system_subtask_status |= flag;
}
u8 system_status_get(void){
	return system_status;
}
/********************************************************************
 * Extern Variables (Extern /Global)
 ********************************************************************/
 
/********************************************************************
 * Local Variables:  STATIC
 ********************************************************************/
static EatEntryPara_st app_para={0};

u8 PcTool_Rx_buf[EAT_UART_RX_BUF_LEN_MAX + 1] = {0};
u8 PcTool_Tx_buf[EAT_UART_TX_BUF_LEN_MAX + 1] = {0};

u8 GPS_Rx_buf[EAT_UART_RX_BUF_LEN_MAX + 1] = {0};
u8 GPS_Tx_buf[EAT_UART_TX_BUF_LEN_MAX + 1] = {0};



/********************************************************************
 * External Functions declaration
 ********************************************************************/
extern void APP_InitRegions(void);

/********************************************************************
 * Local Function declaration
 ********************************************************************/
void app_main(void *data);
void app_func_ext1(void *data);
extern void app_modem(void *data);

/********************************************************************
 * Local Function
 ********************************************************************/
#pragma arm section rodata = "APP_CFG"
APP_ENTRY_FLAG 
#pragma arm section rodata

#pragma arm section rodata="APPENTRY"
	const EatEntry_st AppEntry = 
	{
		app_main,
		app_func_ext1,
		(app_user_func)app_modem,//主业务（GSM、OBD）处理
		(app_user_func)EAT_NULL,//预留给OBD业务处理
		(app_user_func)app_linking,//M2M与服务器通讯心跳包
		(app_user_func)app_gps,//GPS业务处理
		(app_user_func)EAT_NULL,//app_user5,
		(app_user_func)EAT_NULL,//app_user6,
		(app_user_func)EAT_NULL,//app_user7,
		(app_user_func)EAT_NULL,//app_user8,
		EAT_NULL,
		EAT_NULL,
		EAT_NULL,
		EAT_NULL,
		EAT_NULL,
		EAT_NULL
	};
#pragma arm section rodata
/*通过位定义各个任务是否启动
*/

void TastStartSet_00(u8 flag){
	if(0 == flag)Ltaskstartflag &= ~(0x01);
	else Ltaskstartflag |= 0x01;
}
void TastStartSet_01(u8 flag){
	if(0 == flag)Ltaskstartflag &= ~(0x02);
	else Ltaskstartflag |= 0x02;
}
void TastStartSet_02(u8 flag){
	if(0 == flag)Ltaskstartflag &= ~(0x04);
	else Ltaskstartflag |= 0x04;
}
void TastStartSet_03(u8 flag){
	if(0 == flag)Ltaskstartflag &= ~(0x08);
	else Ltaskstartflag |= 0x08;
}
void TastStartSet_04(u8 flag){
	if(0 == flag)Ltaskstartflag &= ~(0x10);
	else Ltaskstartflag |= 0x10;
	user_infor("e:GPS OK[%x]",Ltaskstartflag);
	
}
/* C lib function begin*/
#ifdef EAT_CORE_APP_INDEPEND
__value_in_regs struct __initial_stackheap $Sub$$__user_initial_stackheap(unsigned R0, unsigned SP, unsigned R2, unsigned SL)
{
   struct __initial_stackheap config;
 
   config.heap_base = R0;
   config.heap_limit =  R2;
   config.stack_base =  SP; 
   config.stack_limit =   SL;
   return config;
}
void $Sub$$_fp_init(void)
{
}
void $Sub$$_initio(void)
{
}
__value_in_regs struct __argc_argv $Sub$$__ARM_get_argv(void *a )
{
    /*
    int argc;
    char **argv;
    int r2, r3;
    */
    struct __argc_argv arg;
    arg.argc = 0;
    arg.r2 = 0;
    arg.r3 = 0;
    return arg;
}
#endif
/* C lib function end*/

void app_func_ext1(void *data)
{
	/*This function can be called before Task running ,configure the GPIO,uart and etc.
	   Only these api can be used:
		 eat_uart_set_debug: set debug port
		 eat_pin_set_mode: set GPIO mode
		 eat_uart_set_at_port: set AT port
	*/
	eat_uart_set_at_port(EAT_UART_NULL);// UART1 is as AT PORT
  	eat_pin_set_mode(EAT_PIN26_PWM, EAT_PIN_MODE_EINT);
  	eat_pin_set_mode(EAT_PIN24_COL4, EAT_PIN_MODE_GPIO);//默认为GPIO 该中断只有当系统进入睡眠后才启动
}

/**************************************************************************************
初始化串口功能
USART1与 OBD通讯  波特率115200
**************************************************************************************/
static void InitUartConfig(void){
	char debug[32];
	EatUartConfig_st uart_config =
    	{
		EAT_UART_BAUD_115200,
		EAT_UART_DATA_BITS_8,
		EAT_UART_STOP_BITS_1,
		EAT_UART_PARITY_NONE
    	};
	
	if(eat_uart_open(EAT_UART_1) == EAT_FALSE)
	{
		user_debug("i:[%s] uart(%d) open fail!", __FUNCTION__, EAT_UART_1);
	}
	else
	{
		uart_config.baud = EAT_UART_BAUD_115200;
		uart_config.dataBits = EAT_UART_DATA_BITS_8;
		uart_config.parity = EAT_UART_PARITY_NONE;
		uart_config.stopBits = EAT_UART_STOP_BITS_1;
		if(EAT_FALSE == eat_uart_set_config(EAT_UART_1, &uart_config))
		{
			user_debug("i:[%s] uart(%d) set config fail!", __FUNCTION__, EAT_UART_1);
		}
		 eat_uart_set_send_complete_event(EAT_UART_1, EAT_TRUE);
		 user_debug("i:USART2:USART1-init OK");
		 strcpy(debug, "USART1-init OK");
		 eat_uart_write(EAT_UART_1,(const unsigned char *)debug, strlen(debug));
	}
}

/**************************************************************************************
初始化IO 功能

**************************************************************************************/
static void InitIOConfig(void)
{

}

/**************************************************************************************
加载参数设置

**************************************************************************************/
static void InitDataConfig(void)
{

}

void StartTimeCall(EatTimer_enum nmb , unsigned int Time){
	eat_timer_start(nmb, Time);
}
void StopTimeCall(EatTimer_enum nmb){
	eat_timer_stop(nmb);
}
static void InitSysConfig(void){
        EatRtc_st rtc = {0};
	
	StartTimeCall(EAT_TIMER_2 , 1000);
	//StartTimeCall(EAT_TIMER_1 , 2000);  //时钟 每1S产生一次中断
	
	rtc.year = 12;
	rtc.mon = 10;
	rtc.day = 10;
	rtc.hour = 11;
	rtc.min = 11;
	rtc.sec = 59;
	eat_set_rtc(&rtc);
}


static void event_timer(const EatEvent_st* event){

	EatTimer_enum time = event->data.timer.timer_id;
       EatRtc_st rtc = {0};
		
	if(time == EAT_TIMER_2)
	{
		StartTimeCall(EAT_TIMER_2 , 1000);
		eat_send_msg_to_user(EAT_USER_0, EAT_USER_3, EAT_FALSE, 6, "TIME1", EAT_NULL);
		//user_debug("EAT_TIMER_2 1s");
	}
	if(time == EAT_TIMER_1)
	{
		//StartTimeCall(EAT_TIMER_1 , 2000);
		 user_infor("e:EAT_TIMER_1 2s");
		 eat_get_rtc(&rtc);
		 user_infor("e:Timer test 4 get rtc value:%d/%d/%d %d:%d:%d",rtc.year,rtc.mon,rtc.day,rtc.hour,rtc.min,rtc.sec);
	}
}
void app_handle(void)
{
	eat_modem_write("ATD112;",7);
}

u8 uart_read(const EatEvent_st* event){
	if(event->data.uart.uart == EAT_UART_1){
		obd_read();
		app_sleep_flag_set(0);
	}
	/*else if(event->data.uart.uart == EAT_UART_2){
		if(gps_read()){
			 app_sleep_flag_set(0);
			 if(0x10 == (Ltaskstartflag & 0x10))eat_send_msg_to_user(EAT_USER_0, EAT_USER_4, EAT_FALSE, 4, "GPS", EAT_NULL);
		}
	}*/
	return 0;
}

void test_handler_int_edge(EatInt_st *interrupt)
{
    eat_trace("test_handler_int_edge interrupt->pin [%d ],interrupt->level=%d",interrupt->pin,interrupt->level);
    if(interrupt->level)
    {
        //Rising edge trigger,and set falling trigger
       eat_int_set_trigger(interrupt->pin, EAT_INT_TRIGGER_FALLING_EDGE);
       eat_trace("System interrupt by Einterrupt->pin [%d ] TRIGGER_RISING_EDGE",interrupt->pin);
    }
    else
    {
        //Falling trigger
        eat_int_set_trigger(interrupt->pin, EAT_INT_TRIGGER_RISING_EDGE);
        eat_trace("System interrupt by interrupt->pin [%d ] TRIGGER_FALLING_EDGE",interrupt->pin);        
    }
}

void test_handler_int_level(EatInt_st *interrupt)
{
    eat_trace("test_handler_int_level interrupt->pin [%d ],interrupt->level=%d",interrupt->pin,interrupt->level);
    if(!(interrupt->level))
    {
        //high level trigger,and set low level trigger
       eat_int_set_trigger(interrupt->pin, EAT_INT_TRIGGER_HIGH_LEVEL);
       eat_trace("System interrupt by interrupt->pin [%d ]TRIGGER_LOW_LEVEL",interrupt->pin);
    }
    else
    {
        //low level trigger
        eat_int_set_trigger(interrupt->pin, EAT_INT_TRIGGER_LOW_LEVEL);
        eat_trace("System interrupt by interrupt->pin [%d ] TRIGGER_HIGH_LEVEL",interrupt->pin);        
    }
}

/**************************************************************************************
主程序

**************************************************************************************/
extern void MDM_ADCWARMENABLE(void);
void app_main(void *data)
{
	EatEvent_st event;
	EatEntryPara_st *para;
	u32 eventnum,eventnumindex;
	u8 u8temp;

	//user_debug_unable();
	
	APP_InitRegions();					//Init app RAM
#ifdef EAT_CORE_APP_INDEPEND
	__rt_lib_init(0xF03F8000,0xF0400000); //Init C lib
#endif

	para = (EatEntryPara_st*)data;
  
  	Ltaskstartflag = 0;
  	system_status = 0;
  
	InitUartConfig();

	InitIOConfig();

	InitDataConfig();

	InitSysConfig();
	
	
	user_debug_enable();
	bigmem_init();
	db_init();//初始化数据参数
	mdm_Init();
	obd_init();
	gps_Init();
	Proticol_Init();
	linking_init();
	//anydata_init();     					
	memcpy(&app_para, para, sizeof(EatEntryPara_st));
	user_infor("e:App Main ENTRY  update:%d result:%d", app_para.is_update_app,app_para.update_app_result);
	if(app_para.is_update_app && app_para.update_app_result)
	{
		eat_update_app_ok();
	}
	user_debug_enable();      //add by lilei
	user_debug("i:Car-Eye-DTU-(V02.00)-0403\r\n");
	
	//app_handle();
	TastStartSet_00(1);

	eat_sleep_enable(EAT_FALSE);
	u8temp = 0;
	eat_lcd_light_sw(EAT_FALSE, 0);
	eat_kpled_sw(EAT_FALSE);
	eat_sleep_enable(EAT_TRUE);
	
	
	//中断初始化 下降沿触发
	//eat_int_setup(EAT_PIN26_PWM, EAT_INT_TRIGGER_HIGH_LEVEL, 10, test_handler_int_level);
	eat_gpio_setup(EAT_PIN26_PWM, EAT_GPIO_DIR_INPUT, /*EAT_GPIO_LEVEL_LOW*/EAT_GPIO_LEVEL_HIGH);	
	eat_int_setup(EAT_PIN26_PWM, EAT_INT_TRIGGER_FALLING_EDGE/*EAT_INT_TRIGGER_RISING_EDGE*/, 10, NULL);    //add by lilei-2016-05-11
	//
     

	/*//2015/12/18 16:48 方萃松 改到睡眠前设置该中断
	eat_gpio_setup(EAT_PIN24_COL4, EAT_GPIO_DIR_INPUT, EAT_GPIO_LEVEL_HIGH);
	eat_int_setup(EAT_PIN24_COL4, EAT_INT_TRIGGER_LOW_LEVEL, 50, NULL);
	eat_pin_set_mode(EAT_PIN24_COL4, EAT_PIN_MODE_GPIO);//设置为普通IO口 关闭中断
	*/

	
	while(EAT_TRUE)
	{
		MDM_writeEx();//GSM业务处理数据发送到modem
		if(1 == system_status && 0x55 == system_status_flag)
		{
			system_status_flag = 0;
			user_debug("i:M2M to sleep\r\n");
			MDM_writeEx();//进入系统之前发送完缓冲区的数据
			system_subtask_status = 0;
			eat_send_msg_to_user(EAT_USER_0, EAT_USER_4, EAT_FALSE, 1, "S", EAT_NULL);
			eat_send_msg_to_user(EAT_USER_0, EAT_USER_1, EAT_FALSE, 1, "S", EAT_NULL);
   		}
   		else if(system_subtask_status >= 0x09)
		{
		  
			hw_th_unable_set(0x7f);//启动防盗
			db_obd_save();
			StopTimeCall(EAT_TIMER_2);
			eventnum = eat_get_event_num();
			for(u8temp = 0; u8temp < eventnum; u8temp ++)eat_get_event(&event);
			eat_sleep(10);
			user_debug("i:M2M sleep\r\n");
			//睡眠前设置中断
			eat_gpio_setup(EAT_PIN24_COL4, EAT_GPIO_DIR_INPUT, /*EAT_GPIO_LEVEL_LOW*/EAT_GPIO_LEVEL_HIGH);
	    		eat_int_setup(EAT_PIN24_COL4, EAT_INT_TRIGGER_LOW_LEVEL/*EAT_INT_TRIGGER_FALLING_EDGE*/, 50, NULL);
	    		eat_pin_set_mode(EAT_PIN24_COL4, EAT_PIN_MODE_GPIO);//设置为普通IO口 关闭中断
			eat_pin_set_mode(EAT_PIN24_COL4, EAT_PIN_MODE_EINT);//启动掉电中断
			eat_lcd_light_sw(EAT_FALSE, 0);
	    		eat_kpled_sw(EAT_FALSE);
	    		AT_CFUN(0);
			eat_sleep_enable(EAT_TRUE);
			u8temp = 0;
			while(1)
			{
			   	eat_get_event(&event);
			  	if(event.event == EAT_EVENT_INT)
			   	{
			   	 	eat_pin_set_mode(EAT_PIN24_COL4, EAT_PIN_MODE_GPIO);//设置为普通IO口 关闭中断
			   	 	InitUartConfig();
			   	 	bigmem_init();
	         		 	db_init();//初始化数据参数
	         		 	mdm_Init();
	         		 	obd_init();
	         		 	gps_Init();
	         		 	linking_init();
			   	 	    //anydata_init();              //add by lilei-2016-0823
			   	 	MDM_read();
			   	       user_debug_unable();
			   	 	break;
			   	}
		  	}
		  	system_status = 0;
		  	system_subtask_status = 0;
		  	StartTimeCall(EAT_TIMER_2 , 1000);
		  	eat_sleep_enable(EAT_FALSE);
		  	user_debug("i:i weak up\r\n");
		  
		  	AT_CFUN(1);
      			eat_send_msg_to_user(EAT_USER_0, EAT_USER_4, EAT_FALSE, 1, "U", EAT_NULL);
		  	eat_send_msg_to_user(EAT_USER_0, EAT_USER_1, EAT_FALSE, 1, "U", EAT_NULL);
		}
		eventnum = eat_get_event_num();
		if(eventnum)
		{
			for(eventnumindex = 0; eventnumindex < eventnum; eventnumindex ++)
			{
		   		eat_get_event(&event);
		   //user_debug("wakeup event>>:%d", event.event);
		   //user_debug("%s-%d:msg %x", __FILE__, __LINE__,event.event);
		   		switch(event.event)
				{
					case EAT_EVENT_TIMER :
					event_timer(&event);
					break;

					case EAT_EVENT_KEY:
					break;

					case EAT_EVENT_INT:
					//user_debug("<<<<EAT_EVENT_INT;%d]");
					break;

					case EAT_EVENT_MDM_READY_RD:
					MDM_read();
					//eat_send_msg_to_user(EAT_USER_0, EAT_USER_1, EAT_FALSE, 2, "MR", EAT_NULL);
					break;

					case EAT_EVENT_MDM_READY_WR:
					MDM_writeEnable();
					break;

					case EAT_EVENT_MDM_RI:
					break;

					case EAT_EVENT_UART_READY_RD:
					uart_read(&event);//GPS任务必须已经启动
					break;

					case EAT_EVENT_UART_READY_WR:
					break;

					case EAT_EVENT_ADC:
					break;

					case EAT_EVENT_UART_SEND_COMPLETE :
					break;

					case EAT_EVENT_USER_MSG:
					break;

					default:
					break;
				}
		  	}
		}
		else eat_sleep(5);
	}
}

