

#include "includes.h"

//////////////////////////////////////////////////////////////////////////
// 工况与算法关系对应表
//////////////////////////////////////////////////////////////////////////
// 工况对应算法
const char UP_I_gConditionToArithmetic[] =
{
	0x01,	0x19,	0x02,	0x03,	0x04,	0x05,	0x05,	0x05,	0x05,	0x06,	// 1~10
	0x07,	0x08,	0x07,	0x09,	0x04,	0x0a,	0x03,	0x0b,	0x0c,	0x0d,	// 11~20
	0x0d,	0x0d,	0x0d,	0x0d,	0x0d,	0x0d,	0x0d,	0x0e,	0x0f,	0x00,	// 21~30
	0x10,	0x00,	0x10,	0x11,	0x12,	0x13,	0x13,	0x13,	0x13,	0x13,	// 31~40
	0x13,	0x13,	0x13,	0x03,	0x05,	0x03,	0x03,	0x07,	0x10,	0x14,	// 41~50
	0x07,	0x13,	0x13,	0x13,	0x13,	0x13,	0x13,	0x13,	0x13,	0x15,	// 51~60
	0x15,	0x15,	0x15,	0x00,	0x00,	0x16,	0x17,	0x13,	0x03,	0x04,	// 61~70
	0x03,	0x03,	0x03,	0x03,	0x03,	0x03,	0x18,	0x18,	0x02,	0x05,	// 71~80
	0x05,	0x1a,	0x1a,	0x1a,	0x1a,	0x1b,	0x1b,	0x1b,	0x1b,			// 81~89
};

//////////////////////////////////////////////////////////////////////////
// 算法相关定义
//////////////////////////////////////////////////////////////////////////
static float UP_IArithmetic_1(unsigned char* data);
// 燃油系统状态(0x2103,0x214F) 1:OL;2:CL;3:OL-Drive;4:OL_Fault;5:CL_Fault; else:N/A
static float UP_IArithmetic_2(unsigned char* data);
static float UP_IArithmetic_3(unsigned char* data);
static float UP_IArithmetic_4(unsigned char* data);
static float UP_IArithmetic_5(unsigned char* data);
static float UP_IArithmetic_6(unsigned char* data);

static float UP_IArithmetic_7(unsigned char* data);
static float UP_IArithmetic_8(unsigned char* data);
static float UP_IArithmetic_9(unsigned char* data);
static float UP_IArithmetic_10(unsigned char* data);
// 二次空气请求状态(0x2112) 1:UPS; 2:DNS; 3:OFF; else:undefine
static float UP_IArithmetic_11(unsigned char* data);
/**
0:  B1S----B2S----
1:  B1S1---B2S----
2:  B1S-2--B2S----
3:  B1S--3-B2S----
4:  B1S---4B2S----
5:  B1S----B2S1---
6:  B1S----B2S-2--
7:  B1S----B2S--3-
8:  B1S----B2S---4
*/
static float UP_IArithmetic_12(unsigned char* data);

static float UP_IArithmetic_13(unsigned char* data);
/** 
1:OBD2; 02:OBD; 3:OBD&OBD2; 4:OBD1;
5:NOT OBD; 6:EOBD; 7:EOBD&OBD2; 8:EOBD&OBD;
9:EOBD,OBD&OBD2; 10:JOBD;	11:JOBD&OBD2; 12:JOBD&EOBD;
13:JOBD,EOBD&OBD2; 14:EOBD IV B1; 15:EURO V B2; 16:EURO C;
else: undefine;
*/
static float UP_IArithmetic_14(unsigned char* data);

/**
0  S1B----S2B----
1  S1B1---S2B----
2  S1B----S2B1---
3  S1B-2--S2B----
4  S1B----S2B-2--
5  S1B--3-S2B----
6  S1B----S2B--3-
7  S1B---4S2B----
8  S1B----S2B---4
*/
static float UP_IArithmetic_15(unsigned char* data);

static float UP_IArithmetic_16(unsigned char* data);
static float UP_IArithmetic_17(unsigned char* data);
static float UP_IArithmetic_18(unsigned char* data);

static float UP_IArithmetic_19(unsigned char* data);
static float UP_IArithmetic_20(unsigned char* data);
static float UP_IArithmetic_21(unsigned char* data);
static float UP_IArithmetic_22(unsigned char* data);
static float UP_IArithmetic_23(unsigned char* data);

// h:return / 60; m return % 60
static float UP_IArithmetic_24(unsigned char* data);

static float UP_IArithmetic_25(unsigned char* data);
static float UP_IArithmetic_26(unsigned char* data);
static float UP_IArithmetic_27(unsigned char* data);

// 解析工况需要的算法数组
typedef struct UP_SArithmetic
{
	unsigned char	data_number;
	float (*func)(unsigned char* data); 
}UP_SArithmetic;

// 工况解析算法数组
const struct UP_SArithmetic UP_I_garithmetic[] = 
{
	{1, UP_IArithmetic_1}, {1, UP_IArithmetic_2}, {1, UP_IArithmetic_3}, 
	{1, UP_IArithmetic_4}, {1, UP_IArithmetic_5},	{1, UP_IArithmetic_6},
	{1, UP_IArithmetic_7},	{2, UP_IArithmetic_8},	{1, UP_IArithmetic_9},	
	{2, UP_IArithmetic_10}, {1, UP_IArithmetic_11},	{1, UP_IArithmetic_12},
	{1, UP_IArithmetic_13}, {1, UP_IArithmetic_14},	{1, UP_IArithmetic_15}, 
	{2, UP_IArithmetic_16},	{2, UP_IArithmetic_17}, {2, UP_IArithmetic_18},
	{2, UP_IArithmetic_19}, {2, UP_IArithmetic_20},	{2, UP_IArithmetic_21}, 
	{2, UP_IArithmetic_22},	{2, UP_IArithmetic_23}, {2, UP_IArithmetic_24},
	{2, UP_IArithmetic_25}, {2, UP_IArithmetic_26},	{2, UP_IArithmetic_27}
};
// 算法总个数
const short	UP_I_garithmetic_count = 27;



//////////////////////////////////////////////////////////////////////////
// 算法相关函数实现
//////////////////////////////////////////////////////////////////////////

float UP_IArithmetic_1(unsigned char* data)
{
	if(data[0] < 0x80)
		return (float)data[0];
	else
		return (float)(data[0] - 0x80);
}

float UP_IArithmetic_2(unsigned char* data)
{
	if((data[0] & 0x01) == 1)
		return (float)1;
	else if((data[0] >> 1 & 0x01) == 1)
		return (float)2;
	else if((data[0] >> 2 & 0x01) == 1)
		return (float)3;
	else if((data[0] >> 3 & 0x01) == 1)
		return (float)4;
	else if((data[0] >> 4 & 0x01) == 1)
		return (float)5;

	return (float)0;
}

float UP_IArithmetic_3(unsigned char* data)
{
	return (float)(data[0] * 100.0 / 255);
}

float UP_IArithmetic_4(unsigned char* data)
{
	return (float)(data[0] - 40);
}

float UP_IArithmetic_5(unsigned char* data)
{
	return (float)(data[0] * 199.2 / 255 - 100.0);
}

float UP_IArithmetic_6(unsigned char* data)
{
	return (float)(data[0] * 3);
}

float UP_IArithmetic_7(unsigned char* data)
{
	return (float)(data[0]);
}

float UP_IArithmetic_8(unsigned char* data)
{
	return (float)((data[0] * 0x100 + data[1])/4);
}

float UP_IArithmetic_9(unsigned char* data)
{
	return (float)(data[0] * 127 / 255 - 64);
}

float UP_IArithmetic_10(unsigned char* data)
{
	return (float)((data[0] * 0x100 + data[1]) * 0.01);
}

float UP_IArithmetic_11(unsigned char* data)
{
	if((data[0] & 0x01) == 1)
		return (float)1;
	else if((data[0] >> 1 & 0x01) == 1)
		return (float)2;
	else if((data[0] >> 2 & 0x01) == 1)
		return (float)3;

	return (float)0;
}

float UP_IArithmetic_12(unsigned char* data)
{
	switch(data[0])
	{
	case 0x00:
		return (float)0;
	case 0x01:
		return (float)1;
	case 0x02:
		return (float)2;
	case 0x04:
		return (float)3;
	case 0x08:
		return (float)4;
	case 0x10:
		return (float)5;
	case 0x20:
		return (float)6;
	case 0x40:
		return (float)7;
	case 0x80:
		return (float)8;
	default:
		return (float)0;
	}
}

float UP_IArithmetic_13(unsigned char* data)
{
	return (float)(data[0] * 1.275 / 255);
}

float UP_IArithmetic_14(unsigned char* data)
{
	return (float)(data[0]);
}

float UP_IArithmetic_15(unsigned char* data)
{
	switch(data[0])
	{
	case 0x00:
		return (float)0;
	case 0x01:
		return (float)1;
	case 0x02:
		return (float)2;
	case 0x04:
		return (float)3;
	case 0x08:
		return (float)4;
	case 0x10:
		return (float)5;
	case 0x20:
		return (float)6;
	case 0x40:
		return (float)7;
	case 0x80:
		return (float)8;
	default:
		return (float)0;
	}
}

float UP_IArithmetic_16(unsigned char* data)
{
	return (float)(data[0] * 0x100 + data[1]);
}

float UP_IArithmetic_17(unsigned char* data)
{
	return (float)((data[0] * 0x100 + data[1]) * 5177.265 / 0xffff);
}

float UP_IArithmetic_18(unsigned char* data)
{
	return (float)((data[0] * 0x100 + data[1]) * 10);
}

float UP_IArithmetic_19(unsigned char* data)
{
	return (float)((data[0] * 0x100 + data[1]) * 1.999 / 0xffff);
}

float UP_IArithmetic_20(unsigned char* data)
{
	if(data[0] <= 0x7f)//0x00 <= data[0] && 
		return (float)((data[0] * 0x100 + data[1]) * 8191.75 / 0x7fff);
	else
		return (float)(((data[0] - 0x80) * 0x100 + data[1]) * 8191.75 / 0x7fff - 8192);
}

float UP_IArithmetic_21(unsigned char* data)
{
	return (float)((data[0] * 0x100 + data[1]) * 0.1 - 40);
}

float UP_IArithmetic_22(unsigned char* data)
{
	return (float)((data[0] * 0x100 + data[1]) * 0.001);
}

float UP_IArithmetic_23(unsigned char* data)
{
	return (float)((data[0] * 0x100 + data[1]) * 29126.7 / 0xffff);
}

float UP_IArithmetic_24(unsigned char* data)
{
	return (float)(data[0] * 256 + data[1]);
}

float UP_IArithmetic_25(unsigned char* data)
{
	
	unsigned short shCode = 0;
	memcpy(&shCode, data, sizeof(shCode));
	return (float)shCode;
}

float UP_IArithmetic_26(unsigned char* data)
{
	return (float)((data[0] * 0x100 + data[1]) * 7.995 / 0xffff);
}

float UP_IArithmetic_27(unsigned char* data)
{
	return (float)((data[0] * 0x100 + data[1]) * 255.996 / 0xffff - 128.0);
}


bool PID_getRealVal(u8 type,u8* data,float* val)
{
	float fvalue = 0.0;
	char arithmeticnum = UP_I_gConditionToArithmetic[type - 1];
	// 若对应的算法不在设定范围内
	if(arithmeticnum < 1 || arithmeticnum > 27)
	{
		// 释放内存，返回错误//
		return FALSE;
	}

	fvalue = UP_I_garithmetic[arithmeticnum - 1].func(data);
	*val = fvalue;
	return TRUE;
}

