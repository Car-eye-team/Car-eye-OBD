
/*
*************************************修改日志**********************************

*******************************************************************************
*/
 

#ifndef _COMMON_TOOLS_H
#define _COMMON_TOOLS_H
/*
*******************************************************************************
* 常量定义
*******************************************************************************
*/
#define 	UART_DEBUG_NUM			UART_TXH_2//UART_BLUETOOTH_1//UART_TXH_2
#define    DEBUG_TRACE_EN			1
#define     MSG_TYPE_TO_OBD       		0
#define     MSG_TYPE_TO_PRO      		1

#define	MAX_MSG_TYPE				2
#define	MAX_MSGSIZE					10

/*送给OBD的消息定义*/
#define    MSG_OBD_CLAERDTC			0x01		//清除故障码//
#define    MSG_OBD_READDTC			0x02		//读取故障码//
#define    MSG_OBD_FREEDTC			0x03		//冻结帧//
#define    MSG_OBD_VIN				0x04		//VIN码读取//

/*
*******************************************************************************
* 类型定义
*******************************************************************************
*/

/*
*******************************************************************************
* 全局变量声明
*******************************************************************************
*/
extern u8 MsgQueue[MAX_MSG_TYPE][MAX_MSGSIZE];
/*
*******************************************************************************
* 函数原型声明
*******************************************************************************
*/
void Trace(s8 * pStr);
void TraceHexStr(u8 * pStr, u16 ulen);
void TraceHexStrN(u8 * pStr, u8 ulen);
void InitMsg(void);
s16  PostMsg(u8 MsgType, u16 Msg);
s16  GetMsg(u8 MsgType, u16 * Msg);
void ClearMsg(u8 MsgType, u16 Msg);
s16  AsciiToByte(const u8 * psrc, u8 * pdst );
u16  ByteToAscii( const u8 isrc );
s16  ByteToStr(u8 * pStr, u8 bByte);
s8   IsValidPhoneNum( u8 * const pn );
s16  AsciiToHex( u8 isrc);
s8   HexToAscii(u8 hex);
void SystemInfo( s8 * title, s8 * info, u16 len, u8 flag );
void TimeDly(u16 timeout);
s16  Ascii_2_Hex(s8  *O_data, s8  *N_data, s16 len);
s16  Hex_2_Ascii(s8  *data, s8  *buffer, s16 len);
u8   GetXorSum(u8 *psrc, s16 len);
void StringToBcd(s8 * dst,s8 * src,u8 dstlen, u8 keyc);
u8 	 DelAsciiF(u8 *Fbuf, u8 len);
u8 	 AddAsciiF(u8 *Fbuf, u8 len);
s8   *cm_strstr(s8 *s1, s8 *s2);
s16  MyMemCopy(u8 *dest, u8 *src, u8 stopchar, s16 maxlen);
s32  MyPrintf(s8 * format, ...);
//s32 MyPrintf_tq(s8 * format, ...);

s32 StrPrintf(s8* Dec,s8 * format, ...);

double my_atof(const char* str);

#endif /* _COMMON_TOOLS_H */


