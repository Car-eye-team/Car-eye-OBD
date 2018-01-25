/* car-eye车辆管理平台 
 * car-eye车辆管理公共平台   www.car-eye.cn
 * car-eye开源网址:  https://github.com/Car-eye-admin
 * Copyright car-eye 车辆管理平台  2017 
 */

#ifndef __OBDII_H
#define __OBDII_H

#define ENTERSYS_NEVER		0
#define ENTERSYS_OK		1
#define ENTERSYS_NEVER_SLEEP	2
#define ENTERSYS_OK_SLEEP	4


#define ONEHOUR_CALC(t)		((float)(1000.0/t)*60*60)	//计算瞬时油耗用//

#define LINKING_STATUS_NONE	0x00		//上电前扫描//
#define LINKING_STATUS_OK		0x01		//总线初始化成功（表示已进入ECU）//
#define LINKING_STATUS_FAIL		0x02		//总线正在初始化（表示尝试进入ECU）//

#define ENGER_STATUS_NONE		0x00		//未知//
#define ENGER_STATUS_ON		0x01		//引擎启动(点火)//
#define ENGER_STATUS_OFF		0x02		//引擎未启动(熄火)//



#define MAX_SPEC_NUM	3

#define MAX_DTC_NUM		51


#define CALCOIL_NULL_TYPE	0	//不支持油耗//
#define CALCOIL_MAF_TYPE	1	//空气流量方式//
#define CALCOIL_MAP_TYPE	2	//进气歧管绝对压力方式//
#define CALCOIL_ABS_TYPE	3	//绝对负荷方式//


#define APP_READSTEP_NULL		0x00		//无步骤//
#define APP_READSTART_STEP	0x01		//读取//
#define APP_READING_STEP		0x02		//读取中 //

#define APP_READEND_STEP		0x04		//读取完成//

#define MIL_READ_EN				1
#define MIL_READ_DIS			2

#define OBD_CMD					0x0165
#define HAND_SHANKE				0x0001
#define CMD_FIRE_ON				0x0002
#define CMD_FIRE_OFF			0x0003
#define CMD_DTC					0x0004
#define JIA_SU_FLAG				0x0005
#define JIAN_SU_FLAG			0x0006
#define CMD_STREAM				0x0007
#define VIN_MA					0x0008
#define CMD_TRAVEL_STATUS		0x0009
#define RETURN_DTC				0x000A
#define RETURN_CLEAR_DTC		0x000B
#define M_ID					0x000C
#define CAR_STATUS              0x000D
#define GSM_Power_OFF_Relay		0x000E
#define GPS_DATA_SEND			0x000F



#pragma pack(1)
typedef struct
{
	u8 step;
	u32 time;
}_sCtrlObd;

typedef struct
{
	bool flag;
	u8 Cmd[8];
	u8 len;
}_sCmdRead;

typedef struct
{
	u32 jiffies;	//里程统计计时//
	float CmMile;
 	u8 CmCnt;
}_sVssMile;

typedef struct
{
	bool flag;			//数据更新标志//
	u32 jiffies;	//油耗计算时间//
}_sOilCalc;


typedef struct
{
	bool flag;
	u32 time;//行程时间//
	u32 mile;//行程里程//
	float oil;//行程耗油量//
	float boil;//行程100公里平均油耗//

	u32 idletime;//怠速时间//
	u8 hspeed;//最高速度//
	u8 pspeed;//平均速度//
	u8 jiascnt;//急加速次数//
	u8 jianscnt;//急减速次数//
}sTrip;


typedef struct
{
   u32 cnt;
   u32 times;
   u32 miles;
   u32 tempmiles;
   float oils;
   float tempoils;
   float boils;

}sTrips;


//急踩刹车加速度-35m/ss//
//急加油以0~100KM/H提速为标准，为10m/ss//
typedef struct
{
	bool jiasu_fg;
	bool jiansu_fg;
	u8 oldspeed;
	float oldrpm;
	float tripmile;
}_sgctrl;

typedef struct
{
	u8 time;
	u8 Name;	//数据类型//
	u8 Val[4];	//值//
}_sPidtab;




typedef struct
{
	u8 PidDtcNum;	//故障灯亮指示的故障数//
	bool fg;		//读取标志//
	u8 Num;			//故障码个数//
	u8 dtc[MAX_DTC_NUM][5];//故障码//
}_sDTC;

typedef struct
{
	bool SpecData;
	u8 CmdNum;
	u8 Cmd[MAX_SPEC_NUM];
	bool Piddata;
}_sReadType;

typedef struct
{
	bool MIL_fg;		//MIL灯状态//
	u8 DTCs;			//当前故障个数//
	u8 Misfire;
	u8 Catalyst1;
	u8 Catalyst2;
	//_sDTC DtcBuf;
}_sInessCode;

typedef struct
{
	u8 name;
	u8 Val[2];	//值//
}_sFreezePid;

typedef struct			//冻结帧//
{
	bool readpidfg;	//是否需读取冻结的数据流//
	bool Fg;	//存在冻结帧标志//
	u8 Dtc[5];		//冻结的故障//
//	u8 FreePidNum;	//冻结的数据流个数//
	u8 sup1[4];//支持字1//
	u8 sup2[4];//支持字2//
	u8 sup3[4];//支持字3//
	
	u16 Savecnt;	//数据长度//
	_sFreezePid Pid[128];	//冻结的数据流//
}_sFreezeDtc;


typedef struct
{
	bool Fg;
	u8 MC_vin;	//MessageCount VIN = ?  response messages//
	u8 Data[17];	////
}_sVIN;

typedef struct
{
	u8 Freeze_Fg:1;
	u8 CLearDtc_Fg:1;
	u8 ReadDtc_Fg:1;
	u8 ReadVin_Fg:1;

	u8 res:4;
}_sCOMCMD;

typedef struct
{
	_sCOMCMD cmd;
	u32 time;
}_sCOM;

typedef struct
{
	u8 Data[200];	//Pid数据，最长198字节//
	u32 PidSup1;
	u32 PidSup2;
	u32 PidSup3;
	

}_sPid;




typedef struct
{
	u8 oilcalctype;	//油耗计算方式//
	u8 vss;		//车速  0x0D//
	s8 fireangle;  	//点火提前角0x0e//
	u8 RemainL;
	u16 TrouLen;
	u16 CleartrouLen;
	float rpm;	 //转速0x0C//
	float maf;	//空气流量 0x10//
	
	float ect;		//水温 0x05//
	float map;	//进气歧管绝对压力 0x0B//
	float iat;		//进气温度 0x0F//

	float load_abs;	//绝对负荷0x43//
}_sPid_Cycle;


typedef struct
{
	

	_sReadType ReadType;
	bool entry;
	u8 ProtoclType;//协议类型//
	u8 Link_status;//总线状态//
	u8 Enger_status;//引擎状态//

	float EngineL;

	_sPid Pid;
	_sPid_Cycle PidCycle;

	
	_sInessCode InessCode;	//准备就绪代码与故障码//
			//数据流//
	_sFreezeDtc Freeze;	//冻结帧//
	_sVIN Vin;

	float ssOil;	//瞬时油耗//
	float bgOil;	//百公里平均油耗//

	u32 tripmile;	//里程//

	_sCOM com;
	
}_sOBDDATA;


typedef struct
{
	u8 step;	//读取控制标准//

	u8 n;
	u8 x;
	u8 id[256];

	u8 data[512];	//数据缓存//
	u16 len;		//数据长度//
}sAppPidreadCtl;

typedef struct
{
	u8 step;	////
	bool flag;	//有无标志//
	u8 dtc[5];
	u8 data[300];
	u16 len;
}sAppFreezeDtcCtl;	//冻结帧读取//

typedef struct
{
	u8 step;	////
	bool AutoReadFalg;
	_sDTC dtc;
}sAppDtcreadCtl;	//故障码读取//

typedef struct
{
	u8 step;	////
}sAppVinreadCtl;	//故障码读取//


typedef struct
{
	u8 flag;	//是否读取故障灯状态//
	u32 time;	//读取间隔//
}_sMilRead;


typedef struct
{
	float firstMile;	//初始里程//
	float firstOile;		//初始总耗油//
	float first100Oile;	//初始100公里耗油值//
	float first100Mile;	//初始100公里里程值//

	float CurMile;	//当前跑的里程数//
	float CurOil;	//当前耗油数//
}sDtuRdata;



#pragma pack()

extern _sOBDDATA  ObdData;
extern sTrip trip;
extern sTrip temptrip;		//临时行程//
extern sTrips trips;
extern sAppPidreadCtl AppPidreadCtl;
extern sAppFreezeDtcCtl AppFreezeDtcCtl;
extern sAppDtcreadCtl AppDtcreadCtl;
extern sAppDtcreadCtl AppDtcClearCtl;
extern u8 K_Enter_ErrFg;
extern u8 K2000_Enter_ErrFg;


void CmdRead_In(u8* cmd,u8 len);
void Obd_App_Initial(void);
void Obd_task(void);
bool WakeSysInit(void);
void Read_Trip_Data(void);
void Read_Trips_Data(void);
#endif


