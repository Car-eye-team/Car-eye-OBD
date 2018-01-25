
/* car-eye车辆管理平台 
 * car-eye车辆管理公共平台   www.car-eye.cn
 * car-eye开源网址:  https://github.com/Car-eye-admin
 * Copyright car-eye 车辆管理平台  2017 
 */

#ifndef _BSPOBDII_H_
#define _BSPOBDII_H_

//#define ToolExtDeviceScan_en

#define CMD_READ_NULL		0
#define CMD_READ_OK			1
#define CMD_READ_TIMEOUT	2


#define PWM_OUT_MODE    1
#define PWM_IN_MODE     2
#define VPW_OUT_MODE    3
#define VPW_IN_MODE     4

#define PWM_REC_BUF_LEN		2048
#define PWM_SOF_32US		64
#define PWM_SOF_16US		32
#define PWM_DATA_16US		32
#define PWM_DATA_8US		16
#define PWM_EOF_48US		96

//#define PWM_SOF_32US		32
//#define PWM_SOF_16US		16
//#define PWM_DATA_16US		16
//#define PWM_DATA_8US		8

//#define PWM_EOF_48US		48



#define VPW_SOF_200US 		400
#define VPW_1_128US			256
#define VPW_1_64US			128

#define VPW_0_128US			256
#define VPW_0_64US			128

#define VPW_EOF_280US		560


#define PROTOCOL_NONE			0x00
#define PROTOCOL_J1850_PWM		0x01
#define PROTOCOL_J1850_VPW		0x02
#define PROTOCOL_ISO_5BPS		0x03
#define PROTOCOL_KWP_5BPS		0x04
#define PROTOCOL_KWP_25MS		0x05
#define PROTOCOL_CAN_11BIT_500K	0x06
#define PROTOCOL_CAN_29BIT_500K	0x07
#define PROTOCOL_CAN_11BIT_250K	0x08
#define PROTOCOL_CAN_29BIT_250K	0x09
#define ISO_UARTBUF_SIZE			256



#pragma pack(1)
/******************************************/
//J1850//
typedef struct
{
	u8 recbit:1;

	u8 toggle:1;
	u8 res:6;
}_sBit;

typedef struct
{
	u8 step;
	u16 time[PWM_REC_BUF_LEN];
 	u16 head;
 	u16 tail;
 	_sBit bit;
}_sRecP;

typedef struct
{
	u16 tail;
	u16 head;
	u8 time[512];
}_sSendP;

typedef struct
{
	u8 tim3mode;
}sTimctl;

typedef struct
{
	u8 Step;
	u16 StartCCR;
	u16 H_Time;
	u16 L_Time;
	
	bool BitToggle;
	u8 ByteCnt;
	bool MulRec;
	
	u8 DataRecing;
	u8 LenRecing;
	u8 buf[32];
}_sPwmRec;

typedef struct
{
	u8 Step;
	
	bool SendBitStatus;
	bool BitToggle;
	u8 ByteCnt;
	
	u8 DataSending;
	u8 Len;
	u8 SendLen;
	u8 buf[16];
}_sPwmSend;

typedef struct
{
	bool flag;
	u8 Cnt;
	u8 data[16][16];
}_sRecFrame;


typedef struct
{
	bool Status;
	bool DataStatus;
	bool DataOkStatus;
	u32 time;
	
	_sPwmRec rec;
	_sPwmSend send;
	_sRecFrame Frame;
	
}_SPWM;


typedef struct
{
	u8 Step;
	u16 StartCCR;
	u16 H_Time;
	u16 L_Time;
	
	bool BitToggle;
	u8 ByteCnt;
	bool MulRec;
	
	u8 DataRecing;
	u8 LenRecing;
	u8 buf[32];
}_sVpwRec;

typedef struct
{
	u8 Step;
	
	bool SendBitStatus;
	bool BitToggle;
	u8 ByteCnt;
	
	u8 DataSending;
	u8 Len;
	u8 SendLen;
	u8 buf[16];
}_sVpwSend;

typedef struct
{
	bool flag;
	u8 Cnt;
	u8 data[16][16];
}_sVpwRecFrame;


typedef struct
{
	bool Status;
	bool DataStatus;
	bool DataOkStatus;
	u32 time;
	
	_sVpwRec rec;
	_sVpwSend send;
	_sVpwRecFrame Frame;
}_SVPW;



/******************************************/
//ISO9141//
typedef struct
{
	u8 head;
	u8 tail;
	u8 buf[ISO_UARTBUF_SIZE];
}_sISOUartBuf;

typedef struct
{
	bool flag;
	u8 trycnt;
	u8 mode;
	bool status;
	u8 bitcnt;
	u16 bittime;
	u16 StartTime;
	u16 CurTime;
	u8 buf[8];
}_sRateSync;

typedef struct
{
	bool flag;
	bool headflag;
	bool head1flag;
	u8 address;
	u8 Mode;
	u8 Pid;
	u8 recdatalen;
	u8 data;
	u8 checksum;
	u8 buf[32];
	u8 len;
	u8 tlen;
	bool p2startfg;
	u32 p2jiffies;
}_Isorec;

typedef struct
{
	u8 buf[32];
	u8 len;
}_IsoSend;

typedef struct
{
	_sRateSync Sync;
	_Isorec rec;
	_sRecFrame frame;
	_IsoSend send;
	bool DataOkStatus;
	u32 time;
}_SISO9141;

/******************************************/
//综合//

typedef struct
{
	u8 buf[8];
}_sFrame;

typedef struct
{
	u32 address;
	u8 data[255];
	u8 len;
	u8 reclen;
	u8 FrameCnt;
}_sECU;



typedef struct
{
	u8 Protocol_type;
	u32 address;	//CAN 本身ID//
	u8 flag;	//接收标志//
	u8 noans;	//无应答计数//
	u8 cmd[8];
	u8 cmdlen;

	_sECU ecu1;
	_sECU ecu2;
	
}_sbspctl;

#pragma pack()

extern _sbspctl* BspCtl;


//中断函数定义//
void Pwm_Int(void);
void IsoUartInt(void);

void BspObdii_initial(void);
void EntryOkInitial(u8 type);

bool Pwm_EntryLink(void);
bool Vpw_EntryLink(void);
bool Iso9141_EntryLink(void);
//bool Kwp5bps_EntryLink(u16 bps);
bool Kwp25bps_EntryLink(void);
bool CanExt250K_EntryLink(void);
bool CanExt500K_EntryLink(void);
bool CanStd250K_EntryLink(void);
bool CanStd500K_EntryLink(void);

void BspObdiiRead(_sbspctl* bspctl);
bool SendData2Usart3AndReciveKey(u8 ucCount,const u8 *pucSendBuffer);

#endif




