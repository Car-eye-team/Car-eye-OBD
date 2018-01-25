
/*
*************************************修改日志**********************************

*******************************************************************************
*/

#include "Includes.h"

/*
**************************************************************
                 常量定义
***************************************************************
*/
#define MESSAGE_DEBUG           1

/*
*******************************************************************************
* 常量定义
*******************************************************************************
*/
#define		MSG_NULL				0x00			/*消息为空*/

/*
*******************************************************************************
* 类型定义
*******************************************************************************
*/

/*
*******************************************************************************
* 变量定义
*******************************************************************************
*/
u8 MsgQueue[MAX_MSG_TYPE][MAX_MSGSIZE];

/*
*******************************************************************************
* 函数原型定义
*******************************************************************************
*/

/*
******************************************************************************
功能描述：延时函数(以5ms为单位)
输入参数；timeout:延时值
输出参数；无
返回参数：无
备注:     此函数为暂停所有任务执行,所以慎用
******************************************************************************
*/
void TimeDly(u16 timeout)
{
	u32 jiffiesDly;
	u32 timeoutdly;

	GetSysTick((u32*)&jiffiesDly);
	timeoutdly = timeout*5;

	while(1)
	{
		if(CheckSysTick((u32*)&jiffiesDly,timeoutdly))
		{
			break;
		}
		BSP_RESET_DOG();
	}
}

/*
 ******************************************************************************
 * 函数描述: 初始化消息队列
 * 输入参数: 无
 * 输出参数: 无
 * 返回值：无
 ******************************************************************************
 */
void InitMsg(void)
{
	s8 i = 0;
	s8 j = 0;

	for (i=0;i<MAX_MSG_TYPE;i++)
	{
		for (j=0;j<MAX_MSGSIZE;j++)
		{
			MsgQueue[i][j] = 0;
		}
	}
}

/*
 ******************************************************************************
 * 函数描述: 向指定类型的消息队列发送消息的函数
 * 输入参数: MsgType: 消息类型
 *               Msg: 消息内容
 * 输出参数: 无
 * 返回值：0:成功  -1:失败 
 ******************************************************************************
 */
s16 PostMsg(u8 MsgType, u16 Msg)
{
    s8 i;

    if (MsgType >= MAX_MSG_TYPE)
    {
    	return -1;
    }

    //MyPrintf("Msg type:%d, Msg:%d\r\n", MsgType, Msg);
    for(i=0;i<MAX_MSGSIZE;i++)
    {
        if (MsgQueue[MsgType][i] == MSG_NULL)
        {
            MsgQueue[MsgType][i] = Msg;
            return 0;
        }
    }
	Trace("Msg full!\r\n");
    
    return -1;
}

/*
 ******************************************************************************
 * 函数描述: 向指定类型的消息队列发送消息的函数
 * 输入参数: MsgType: 消息类型
 * 输出参数: *Msg: 消息内容
 * 返回值：0:成功  -1:失败 
 ******************************************************************************
 */
s16 GetMsg(u8 MsgType, u16 * Msg)
{
    s8 i;
    
    if (MsgType >= MAX_MSG_TYPE)
    {
    	return -1;
    }
    
    if (MsgQueue[MsgType][0] != MSG_NULL)
    {
        *Msg = MsgQueue[MsgType][0];
        i = 0;
        while(1)
        {
            if (i >= MAX_MSGSIZE-1)
            {
		        MsgQueue[MsgType][i] = MSG_NULL;
		        break;
            }
            if (MsgQueue[MsgType][i] == MSG_NULL)
            {
                break;
            }
            MsgQueue[MsgType][i] = MsgQueue[MsgType][i+1];
            i++;
        }
        return 0;
    }
    
    return -1;
}

void ClearMsg(u8 MsgType, u16 Msg)
{
	s8 i,x;
    
    if (MsgType >= MAX_MSG_TYPE)
    {
    	return;
    }

    i = 0;

    while(1)
    {
    	if (MsgQueue[MsgType][i] == Msg)
    	{
    		MsgQueue[MsgType][i] = MSG_NULL;
    	}

    	i ++;
    	if (i >= MAX_MSGSIZE)
    	{
    		break;
    	}
    }

	i = 0;
	while(1)
	{
		if(MsgQueue[MsgType][i] == MSG_NULL)
		{
			x = i;
			while(1)
			{
				if(MsgQueue[MsgType][x] != MSG_NULL)
				{
					break;
				}
				x++;
				if(x >= MAX_MSGSIZE)
				{
					return;
				}
			}
			
			MsgQueue[MsgType][i] = MsgQueue[MsgType][x];
			MsgQueue[MsgType][x] = MSG_NULL;
			i++;
			if(i >= MAX_MSGSIZE)
			{
				return;
			}
		}
		else
		{
			i ++;
			if(i >= MAX_MSGSIZE)
			{
				return;
			}
		}
	}
}

/*
 ******************************************************************************
 * 功能描述：转换1个数字字符为数字是否为'0~9, a~f, A~F',并判断该字符是否为数字
 *           字符
 * 输入参数：isrc数据
 * 输出参数：无
 * 返回参数：转换后的数据
 *           -1: 出错
 * 外部调用：无
 * 注意事项：无
 ******************************************************************************
 */
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


/*
 ******************************************************************************
 * 功能描述：转换1个数字字符为数字是否为'0~9, a~f, A~F',并判断该字符是否为数字
 *           字符
 * 输入参数：isrc数据
 * 输出参数：无
 * 返回参数：转换后的数据
 *           -1: 出错
 * 外部调用：无
 * 注意事项：无
 ******************************************************************************
 */
s8 HexToAscii(u8 hex)
{
    const s8 Byte2Hex[] = "0123456789ABCDEF";

	return Byte2Hex[hex];
}


/*
 ******************************************************************************
 * 功能描述：将一个字节的数据转换成两个字符表示，如0x6a转换成"6a"存放在16位
             数据中
 * 输入参数：isrc数据
 * 输出参数：无
 * 返回参数：转换后的数据
 * 外部调用：无
 * 注意事项：无
 ******************************************************************************
 */
u16 ByteToAscii( const u8 isrc )
{
	u16 iResult = 0x3030;
	u8 ASCIITAB[] = "0123456789ABCDEF";
	
	
	iResult = (u16)ASCIITAB[ isrc >> 4  ] << 8;
	iResult |= ASCIITAB[ isrc & 0x0f];

	return iResult;
}

/*
 ******************************************************************************
 * 功能描述：将一个字节的数据转换成两个字符表示，如0x6a转换成"6a".
 * 输入参数：bByte
 * 输出参数：pStr
 * 返回参数：0: 成功  -1: 失败
 ******************************************************************************
 */
s16 ByteToStr(u8 * pStr, u8 bByte)
{
    const s8 Byte2Hex[] = "0123456789ABCDEF";
	
	if (NULL == pStr)
	{
		return (s16)-1;
	}
	
	*(pStr++) = Byte2Hex[ bByte >> 4  ];
	*(pStr++) = Byte2Hex[ bByte & 0xF ];
	
	return 0;
}


/*
 ******************************************************************************
 * 功能描述：ASCII 到 HEX的转换函数
 * 输入参数：O_data: 转换数据的入口指针，
 *			N_data: 转换后新数据的入口指针
 *			len : 需要转换的长度
 * 输出参数：无
 * 返回参数：-1: 转换失败
 *			其它：转换后数据长度
 * 注意：O_data[]数组中的数据在转换过程中会被修改。
 ******************************************************************************
 */
s16 Ascii_2_Hex(s8  *O_data, s8  *N_data, s16 len)
{
	int i,j,tmp_len;
	s8  tmpData;
	s8  *O_buf = O_data;
	s8  *N_buf = N_data;
	
	for(i = 0; i < len; i++)
	{
		if ((O_buf[i] >= '0') && (O_buf[i] <= '9'))
		{
			tmpData = O_buf[i] - '0';
		}
		else if ((O_buf[i] >= 'A') && (O_buf[i] <= 'F')) //A....F
		{
			tmpData = O_buf[i] - 0x37;
		}
		else if((O_buf[i] >= 'a') && (O_buf[i] <= 'f')) //a....f
		{
			tmpData = O_buf[i] - 0x57;
		}
		else
		{
			return -1;
		}
		O_buf[i] = tmpData;
	}
	
	for(tmp_len = 0,j = 0; j < i; j+=2)
	{
		N_buf[tmp_len++] = (O_buf[j]<<4) | O_buf[j+1];
	}
	
	return tmp_len;
}

/*
 ******************************************************************************
 * 功能描述：HEX 到 ASCII的转换函数
 * 输入参数：data: 转换数据的入口指针
 *		   buffer: 转换后数据入口指针
 *			 len : 需要转换的长度
 * 返回参数：转换后数据长度
 ******************************************************************************
 */
s16 Hex_2_Ascii(s8  *data, s8  *buffer, s16 len)
{
	const s8 ascTable[17] = {"0123456789ABCDEF"};
	s8  *tmp_p = buffer;
	int i, pos;
	
	pos = 0;
	for(i = 0; i < len; i++)
	{
		tmp_p[pos++] = ascTable[(data[i] >> 4)&0x0f];
		tmp_p[pos++] = ascTable[data[i] & 0x0f];
	}
	tmp_p[pos] = '\0';
	
	return pos;
}
/*
 ******************************************************************************
 * 功能描述：将2个字符串合并成一个字节的数据，并将结果存放在目的地址中
 * 输入参数：psrc 字符串指针，如果输入的字符串非16进制的数据话就返回-1，否则为0
 * 输出参数：转换后的数据
 * 返回参数：-1: 错误
              0: 正确
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

/*
 ******************************************************************************
 * 功能描述：检查是否是有效电话号码字符串
 * 输入参数：被检查的字符串
 * 输出参数：无
 * 返回参数：-1 失败 0 成功
 ******************************************************************************
 */
s8 IsValidPhoneNum( u8 * const pn )
{
    u8 i = 0;
    
    while( '\0' != pn[i] )
    {
        if( ( pn[i] < '0' || pn[i] > '9' ) && ( pn[i] != '*' && pn[i] != '#' ) )
        {
            return (s8)-1;
        }
        else
        {
            ++i;
        }
    }

    if( 0 == i ) /* 空串 */
    {
        return (s8)-1;
    }

    return 0;
}

/*
******************************************************************************
功能描述：打印调试信息，向串口1输出
输入参数；待打印的字符串指针
输出参数；无
返回参数：无
备注:     一次最大打印255个字符或者遇到'\0'字符,超过的丢弃，采用中断发送方式
******************************************************************************
*/
void Trace(s8 *pStr)
{
	#ifdef DEBUG_TRACE_EN
	Uart_send(UART_DEBUG_PRINT, (void *)pStr, strlen((const char*)pStr));
	#endif
}

/*
******************************************************************************
功能描述：以16进制方式打印调试信息，向串口1输出
输入参数；待打印的字符串指针
输出参数；无
返回参数：无
备注:     一次最大打印255个字符或者遇到'\0'字符,超过的丢弃，采用中断发送方式
******************************************************************************
*/
void TraceHexStr(u8 * pStr, u16 ulen)
{
	#ifdef DEBUG_TRACE_EN
	u16 i = 0;
	s8	buf[4];

	for(i=0; i<ulen; i++)
	{
		ByteToStr((u8 *)buf, *pStr);
		buf[2] = ' ';
		buf[3] = 0x00;
		pStr++;
		Trace(buf);
		BSP_RESET_DOG();
	}
	Trace("\r\n");
	#endif
}

/*
******************************************************************************
功能描述：以16进制方式打印调试信息，向串口1输出
输入参数；待打印的字符串指针
输出参数；无
返回参数：无
备注:     一次最大打印255个字符或者遇到'\0'字符,超过的丢弃，采用中断发送方式
******************************************************************************
*/
void TraceHexStrN(u8 * pStr, u8 ulen)
{
	#ifdef DEBUG_TRACE_EN
	u8 i = 0;
	s8	buf[4];

	
	for(i=0; i<ulen; i++)
	{
		ByteToStr((u8 *)buf, *pStr);
		buf[2] = 0x00;
		buf[3] = 0x00;
		pStr++;
		Trace(buf);
		BSP_RESET_DOG();
	}
	Trace("\r\n");
	#endif
}

/*
 ******************************************************************************
 * 功能描述：向 uart1 打印系统信息
 * 输入参数: title, info, len, flag
 * 输出参数: 无
 * 返回值:	 无
 ******************************************************************************
 */
void SystemInfo( s8 * title, s8 * info, u16 len, u8 flag )
{
#ifdef DEBUG_TRACE_EN
	s8	buf[4];
	u16 	i=0;
	u8 *	ptxt;

	ptxt = (u8 *)title;
	Trace((s8 *)ptxt);
	if (0x00 == flag)
	{
		ptxt = (u8 *)info;
		Trace((s8 *)ptxt);
	}
	else if(flag < 0x80)
	{
		ptxt = (u8 *)info;
		BSP_RESET_DOG();
		for (i=0; i<len; i++)
		{
			ByteToStr((u8 *)buf,(u8)*ptxt );
			buf[2] = ' ';
			buf[3] = 0x00;
			ptxt++;
			Trace(buf);
		}
	}
	else
	{
		ptxt = (u8 *)info;
		BSP_RESET_DOG();
		for (i=0; i<len; i++)
		{
			ByteToStr((u8 *)buf, (u8)*ptxt );
			buf[2] = ' ';
			buf[3] = 0x00;
			ptxt--;
			Trace(buf);
		}
	}
	Trace("\r\n");
#endif
}


/*
******************************************************************************
功能描述：格式化输出打印调试信息，向串口1输出
输入参数；待打印的字符串指针
输出参数；无
返回参数：无
备注:     无
******************************************************************************
*/
s32 MyPrintf(s8 * format, ...)
{
 	s32 n = 0;
#ifdef DEBUG_TRACE_EN
 	s8  buf[256];
 	va_list ap;

 	va_start(ap,format);
	n = vsprintf((char *)buf, (const char*)format, ap);
	//Uart_send(2, (void *)buf, n);
	Uart_send(UART_DEBUG_PRINT, (void *)buf, n);
 	va_end(ap);
 #endif
 	return n;
}

/*
s32 MyPrintf_tq(s8 * format, ...)
{
 	s32 n = 0;

 	s8  buf[256];
 	va_list ap;

 	va_start(ap,format);
	n = vsprintf((char *)buf, (const char*)format, ap);
	//Uart_send(2, (void *)buf, n);
	Uart_send(UART_TXH_2, (void *)buf, n);
 	va_end(ap);

 	return n;
}
*/

s32 StrPrintf(s8* Dec,s8 * format, ...)
{
	s32 n;
 	s8  buf[256];
 	va_list ap;
 
 	va_start(ap,format);
	n = vsprintf((char *)buf, (const char*)format, ap);
	memcpy(Dec,(void *)buf, n);
	//BSP_UartSend(UART_DEBUG_NUM, (void *)buf, n);
 	va_end(ap);
 	return n;
}

/*
 ******************************************************************************
 * 功能描述：计算校验和
 * 输入参数：psrc 字符串指针
 *			  len 需要计算的长度
 * 输出参数：无
 * 返回参数：-1: 错误
              0: 正确
 * 外部调用：无
 * 注意事项：无
 ******************************************************************************
 */
u8 GetXorSum(u8 *psrc, s16 len)
{
    s16 i = 0;
    u8 checksum = 0;
    
    for (i=0;i<len;i++)
    {
		BSP_RESET_DOG();
        checksum = checksum ^ psrc[i];
    }

    return checksum;
}
/*
 ******************************************************************************
 * 功能描述：将数字字符串转化为BCD码
 * 输入参数；src,dstlen,keyc
 * 输出参数；dst
 * 返回参数：无
 * 注意事项: 必须是0~9的数字字符，字符串长度小于255
 ******************************************************************************
 */
void StringToBcd(s8 * dst,s8 * src,u8 dstlen, u8 keyc)
{
    u8 i;
    
    i = 0;
    while (dstlen != 0)
    {
        if (((*src) >= '0') && (*src <= '9'))
        {
            dst[i] = *src -0x30;
            src ++;
        }
        else if (((*src) >= 'a') && (*src <= 'f'))
        {
            dst[i] = *src - 'a' + 0x0a;
            src ++;
        }
        else if (((*src) >= 'A') && (*src <= 'E'))
        {
            dst[i] = *src - 'A' + 0x0a;
            src ++;
        }
        else
        {
            dst[i] = keyc;
        }
        
        dst[i] <<= 4;
        
        if (((*src) >= '0') && (*src <= '9'))
        {
            dst[i] |= *src -0x30;
            src ++;
        }
        else if (((*src) >= 'a') && (*src <= 'f'))
        {
            dst[i] |= *src - 'a' + 0x0a;
            src ++;
        }
        else if (((*src) >= 'A') && (*src <= 'E'))
        {
            dst[i] |= *src - 'A' + 0x0a;
            src ++;
        }
        else
        {
            dst[i] |= keyc;
        }
        
        i++;
        dstlen --;
    }
}

/*
 ******************************************************************************
 * 功能描述：将Fbuf中'F'的值填零
 * 输入参数；len
 * 输出参数；Fbuf
 * 返回参数：无
 ******************************************************************************
 */
u8 DelAsciiF(u8 *Fbuf, u8 len)
{
	u8	i;
	
	for(i=len; i>0; i--)
	{
		if(Fbuf[i-1] == 'F')
		{
			Fbuf[i-1]=0;
		}
	}

	return	(len-i+1);
}

/*
 ******************************************************************************
 * 功能描述：将Fbuf中为零的值填'F'
 * 输入参数；len
 * 输出参数；Fbuf
 * 返回参数：无
 ******************************************************************************
 */
u8 AddAsciiF(u8 *Fbuf, u8 len)
{
	u8	i,j;
	
	for(i=0; i<len; i++)
	{
		if(Fbuf[i] == 0)
			break;
	}
	for(j=i; j<len; j++)
	{
		Fbuf[j] = 'F';
	}

	return	i;
}

/*
 ******************************************************************************
 * 功能描述：字符串查找
 * 输入参数：s1 母字符串
 *			 s2 子字符串
 * 输出参数：无
 * 返回参数：子字符串的起始指针
 * 外部调用：无
 * 注意事项：无
 ******************************************************************************
 */
s8  *cm_strstr(s8 *s1, s8 *s2)
{
	//BSP_RESET_DOG();
	return (s8*)strstr((const char*)s1, (const char*)s2);
}

/*
 ******************************************************************************
 * 功能描述：自定义拷贝函数
 * 输入参数：*dest:目的地址
 *			 *src:源地址
 *			 stopchar:停止字符
 *			 maxlen:最大长度
 * 输出参数：无
 * 返回参数：-1:拷贝出错  其它:实际拷贝长度
 * 外部调用：无
 * 注意事项：无
 ******************************************************************************
 */
s16 MyMemCopy(u8 *dest, u8 *src, u8 stopchar, s16 maxlen)
{
	s16 i= 0;

	if ((NULL == dest) || (NULL == src))
	{
		return -1;
	}
	
	for (i=0;i<maxlen;i++)
	{
		*dest++ = *src++;
		if (*src == stopchar)
		{
			break;
		}
	}

	return i;
}

