/* car-eye车辆管理平台 
 * car-eye车辆管理公共平台   www.car-eye.cn
 * car-eye开源网址:  https://github.com/Car-eye-admin
 * Copyright car-eye 车辆管理平台  2017 
 */

#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_


#define COM_TXH_DEAL				1
#define COM_BLUE_DEAL				2

#define TemPertureSign 				0x05
#define TemPertureSign1				(TemPertureSign+1)
#define SpeedSign					0x0d
#define OilSign						0x92
#define DtcSign						0x91
#define DtcSign1						(DtcSign+2)
#define OBD_UPDATE_REQ                    0x8a
#define Reply_OBD_UPDATE     		0x0a
#define BEGIN_OBD_UPDATE    		0x8b
#define UPDATE_BAG_REQ        		0x0c
#define UPDATE_BAG_DOWN     		0x8c
#define UPDATE_OBD_SUCCESS 		0x0d
#define Update_Data_Anser       		0x01
#define Update_Data_Begin        		0x02
#define Update_Data_Getbag      		0x03
#define Update_Data_ReqStatus		0x04
#define UpdataDataLength				512
#define OBDFIREON						1
#define OBDINSUCCESS					2
#define OBDFIREOFF						3
#define OBDBUFFERSIZE					8
#define GSM_POWER_ON			GPIO_WriteBit(GPIOA, GPIO_Pin_5,Bit_SET)
#define GSM_POWER_OFF		    GPIO_WriteBit(GPIOA, GPIO_Pin_5,Bit_RESET)
#define ProBufSize					10
#pragma pack(1)

typedef struct
{
	u8 Com;
	u16 step;  
	u16 cmd;
	u16 len;
	u16 len2;
	u8 buf[1024];
}_ProDeal;

typedef struct 
{
    u16 head;
    u16 tail;
    u8 buf[UART_BUF_SIZE];
} sUartBuf;

typedef struct
{
	bool lock;		//命令锁//
	bool exefg;		//执行标记//
	u32 time;		//锁的控制时间//
	u8 Cmd;
	u8 n;
	u8 buf[256];
	u16 len;
	u8 data[512];
}_sProCtl;


typedef struct
{
   bool flag;
   u16 len;
   u8 buffer[400];

}_SendObdBuffer;
typedef struct
{
   u8 head;
   u8 tail;
   _SendObdBuffer SendObdbuf[OBDBUFFERSIZE];

}_SendDataLoop;


typedef struct
{
   bool HangShakeStatus; 
   bool GsmPowerStatus;
   u8   Obdsatus;
   bool SendFlag;
   u8   AccOffGsmPowerStep;
}_ObdDeal;

typedef struct
{

      u8 ProtocolId;
      u8 ProtocolBuffer[8];

}_ProtocolDeal;


#pragma pack()


extern u8 TempBuf[1024];
extern u8 WaterTemputure;
extern _sProCtl AppCtrl;
extern _ObdDeal ObdDeal;
extern bool GsmPowerOffFlag;
extern _SendDataLoop SendDataLoop;


void Pro_Initial(void);
u16 Pro_Pack(u8* buf,u8* src,u16 len,u8 com);
u16 Pro_Pack_1(u8* buf,u8 Cmd,u8* src,u16 len,u8 com);

void Pro_Send(u8 tag,u8 com);
void Pro_Txh_UartInt(void);

void Pro_Task(void);
void Pro_ClearDtcRes(u8 flag);
u16 Pro_Pack_Obd(u8* buf,u16 cmd1,u16 cmd2);
u16 Pro_Pack_Answer(u8* buf,u16 cmd1,u16 cmd2);
void ObdDealInital(void);
void App_Task(void);
void ObdSendDataDeal(void);
void App_AccOutoCheckACCon(u16 V);
 void SendDataBufClear(void);

#endif

