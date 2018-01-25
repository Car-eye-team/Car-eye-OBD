/* car-eye车辆管理平台 
 * car-eye车辆管理公共平台   www.car-eye.cn
 * car-eye开源网址:  https://github.com/Car-eye-admin
 * Copyright car-eye 车辆管理平台  2017 
 */


 
#ifndef _MAIN_H
#define _MAIN_H
#define VER 			" car-eye-OBDII-1.0.0"


#define CAN_POWER_ON			GPIO_WriteBit(GPIOA, GPIO_Pin_6,Bit_RESET)
#define CAN_POWER_OFF			GPIO_WriteBit(GPIOA, GPIO_Pin_6,Bit_SET)


#pragma pack(1)

typedef struct
{
	u32 flag;
	u8 serialnum[12];
	float Mile;
	float Oil;
	float OilMileage;		//计算平均油耗用//
	float MileageOil;		//计算平均油耗用//
	float EngineL;			//发动机排量//
	u32 Encryptval;

	u32 K_Delay;
	u32 K2000_Delay;
	u32 res3;
	u32 res4;
	u32 res5;
	u32 res6;
	u32 res7;
	u32 res8;
	float oldSetMile;		//上一次校准里程//

	float Mile_Percent;		//里程校准系数//

	u32 end;				///用来作对齐//
}_SaveSet;

#pragma pack()

extern bool accstate;
extern _SaveSet  saveset;
extern float PowerV;
extern u32 SoftDog;


#endif

