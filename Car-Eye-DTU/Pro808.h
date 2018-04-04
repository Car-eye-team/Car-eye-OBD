/* car-eye车辆管理平台 
 * car-eye车辆管理公共平台   www.car-eye.cn
 * car-eye开源网址:  https://github.com/Car-eye-admin
 * Copyright car-eye 车辆管理平台  2018 
 */


#ifndef _PRO_808_
#define _PRO_808_


#define 		UP_UNIRESPONSE			0x0001		//终端通用应答//
#define 		UP_HEARTBEAT				0x0002		//终端心跳//
#define 		UP_REGISTER				0x0100		//终端注册//
#define 		UP_LOGOUT					0x0101		//终端注销//
#define 		UP_AUTHENTICATION		0x0102		//终端鉴权//
#define         UP_POSITIONREPORT             0x0200


#define 		MSGBODY_NOPACKAGE_POS		13
#define 		MSGBODY_PACKAGE_POS			17
#define 		PROTOCOL_SIGN					0x7E		//标识位//
#define 		PROTOCOL_ESCAPE				0x7D		//转义标识//
#define 		PROTOCOL_ESCAPE_SIGN			0x02		//0x7e<-->0x7d后紧跟一个0x02//
#define 		PROTOCOL_ESCAPE_ESCAPE		0x01		//0x7d<-->0x7d后紧跟一个0x01//


#define 		DOWN_UNIRESPONSE				0x8001		//平台通用应答//
#define 		DOWN_REGISTERRSPONSE		0x8100		//终端注册应答//
#define   	OBDBUFFER_SIZE				20
#define 		MAX_PROFRAMEBUF_LEN			512

#define BigLittleSwap16(A)  ((((unsigned short)(A) & 0xff00) >> 8) | \
							(((unsigned short)(A) & 0x00ff) << 8))

#define BigLittleSwap32(A)  ((((unsigned long)(A) & 0xff000000) >> 24) | \
							(((unsigned long)(A) & 0x00ff0000) >> 8) | \
							(((unsigned long)(A) & 0x0000ff00) << 8) | \
							(((unsigned long)(A) & 0x000000ff) << 24))



#define 		   LOOP_BUFFER_SIZE			5

#pragma pack(1)


typedef struct
{
	u8 u8Para1;		//参数一//
	u8 u8Para2;		//参数二//
	u16 u16Para3;	//参数三//
	u16 u16Para4;	//参数四//
	u32 u32Para5;	//参数五//
	u32 u32Para6;	//参数六//
	u8* buf;
}sProPara;

typedef struct
{
   	u8 flag;
   	unsigned short len;
   	unsigned char buffer[400];

}_SendBuffer;
typedef struct
{
   	unsigned head;
   	unsigned char tail;
   	_SendBuffer SendBuf[LOOP_BUFFER_SIZE];

}_SendDataLoop;


typedef union
{
	struct bit
	{
		u16 msglen:10;		//消息体长度//
		u16 encrypt:3;		//数据加密方式// //当此三位都为0，表示消息体不加密//  当第10位为1，表示消息体经过RSA算法加密
		u16 package:1;		//分包标记//
		u16 res1:2;			//保留//
	}bit;
	u16 val;
}sMsgattribute;		//消息体属性格式结构//


typedef struct
{
	u16 id;						//消息ID//
	sMsgattribute  attribute;		//消息体属性//
	u8 phone[6];					//终端手机号//
	u16 serialnum;				//消息流水号//
	u16 totalpackage;				//消息总包数//  // 该消息分包后的总包数//
	u16 packetseq;				//包序号//  //从1开始//
}sMessagehead;		//消息头内容//




typedef union
{
	struct bita
	{
		u32 sos:1;						//紧急报瞥触动报警开关后触发//
		u32 overspeed:1;					//超速报警//
		u32 fatigue:1;						//疲劳驾驶//
		u32 earlywarning:1;				//预警//
		u32 gnssfault:1;					//GNSS模块发生故障//
		u32 gnssantennacut:1;				//GNSS天线未接或被剪断//
		u32 gnssantennashortcircuit:1;		//GNSS天线短路//
		u32 powerlow:1;					//终端主电源欠压//
		
		u32 powercut:1;					//终端主电源掉电//
		u32 lcdfault:1;					//终端LCD或显示器故障//
		u32 ttsfault:1;						//TTS模块故障//
		u32 camerafault:1;					//摄像头故障//
		u32 obddtc:1;						//OBD故障码//
		u32 res1:5;						//保留//

		u32 daydriveovertime:1;			//当天累计驾驶超时//
		u32 stopdrivingovertime:1;			//超时停车//
		u32 inoutarea:1;					//进出区域//
		u32 inoutroad:1;					//进出路线//
		u32 roaddrivetime:1;				//路段行驶时间不足/过长//
		u32 roaddeviate:1;				//路线偏离报警//
		u32 vssfault:1;					//车辆VSS故障//
		u32 oilfault:1;						//车辆油量异常//
		u32 caralarm:1;					//车辆被盗(通过车辆防盗器)//
		u32 caraccalarm:1;				//车辆非法点火//
		u32 carmove:1;					//车辆非法位移//
		u32 collision:1;					//碰撞侧翻报警//
		u32 res2:2;						//保留//
	}bita;
	u32 val;
}sbitalarm;



typedef union
{
	struct bits
	{
		u32 acc:1;						//ACC  0: ACC关;1:ACC开//
		u32 location:1;					//定位  0:未定位;1:定位//
		u32 snlatitude:1;					//0:北纬:1:南纬//
		u32 ewlongitude:1;				//0:东经;1:西经//
		u32 operation:1;					//0:运营状态:1:停运状态//
		u32 gpsencrypt:1;					//0:经纬度未经保密插件加密;l:经纬度已经保密插件加密//
		u32 trip_stat:2;					//00：等待新行程01：行程开始10：正在行驶11：行程结束,(有附带数据0x07)//
		u32 Alarm_en:1;					//防盗功能打开关闭//
		u32 ResetState:1;					//上电状态上报//

		u32 oilcut:1;						//0:车辆油路正常:1:车辆油路断开//
		u32 circuitcut:1;					//0:车辆电路正常:1:车辆电路断开//
		u32 doorlock:1;					//0:车门解锁；1：车门加锁//
		u32 gpsen:1;						// 1:无GPS数据，但字段占用 0：有GPS数据//
		u32 res2:18;						//保留//
	}bits;
	u32 val;
}sbitstate;

typedef struct
{
	sbitalarm alarm;
	sbitstate state;
	u32 latitude;					//纬度(以度为单位的纬度值乘以10的6次方，精确到百万分之一度)//
	u32 longitude;				//经度(以度为单位的纬度值乘以10的6次方，精确到百万分之一度)//
	u16 atitude;					//海拔高度，单位为米(m)//
	u16 speed;					//速度 1/10km/h//
	u16 direction;					//方向 0-359,正北为0，顺时针//
	u8 time[6];					//时间 BCD[6] YY-MM-DD-hh-mm-ss(GMT+8时间，本标准之后涉及的时间均采用此时区)//
}sPositionbasicinfo;


typedef struct
{

   u8  Sim808Step;
   u8  AnsWerFalg;
   u8 AuthenFlag;
   u16 AutionLen;
   u8 AutionBuf[16];

}SIM808DEAL;


#pragma pack()

extern u8 ProTBuf[512];
extern u8 ProTempBuf[512];
extern sProPara  ProPara;
u16 ProFrame_Pack(u8 *dec,u16 Cmd,sProPara* Para,u8* Tempbuf);
u8 SVR808_FameDeal(void);
u8   Up_Register(void);
u8  UP_Authentication(void);
void ProFrameRec(u8 data);
void ProFramePrase(u8* FrameData,u16 Framelen,u16* ResId);

#endif

