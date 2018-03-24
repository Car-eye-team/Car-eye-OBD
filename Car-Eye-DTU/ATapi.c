/**************************
*
*ATÖ¸Áî½Ó¿Ú¶¨Òå
*½Ó¿ÚÃû¶¨Òå¸ñÊ½:AT_XX  xxÎªATÖ¸Áî Èç£ºAT_CCID
*********************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rt_misc.h>

#include "eat_modem.h"
#include "eat_interface.h"
#include "eat_uart.h"
#include "definedata.h"
#include "UartTask.h"
#include "AppUser1.h"
#include "gps.h"
#include "db.h"
#include "ATapi.h"

/*¶¨ÒåTCP/IPÁ´½ÓĞòºÅ0-5
*/
#define TCPIP_MUX_COMM 0x00  //Í¨ÓÃÊı¾İ´«ÊäÁ´½Ó
#define TCPIP_MUX_GPS  0x01  //GPSĞÇÀúÊı¾İÁ´½Ó
#define TCPIP_MUX_CUSTOM 0x02 //¿Í»§·şÎñÆ÷Á´½Ó

#define ATOVERTIME0 1000 //ÉèÖÃÆÕÍ¨ATÖ¸ÁîµÄ³¬Ê±Ê±¼ä
#define ATOVERTIME1 3000 //ÉèÖÃÆÕÍ¨ATÖ¸ÁîµÄ³¬Ê±Ê±¼ä
#define ATOVERTIME2 5000 //ÉèÖÃÆÕÍ¨ATÖ¸ÁîµÄ³¬Ê±Ê±¼ä
#define ATOVERTIME3 7000 //ÉèÖÃÆÕÍ¨ATÖ¸ÁîµÄ³¬Ê±Ê±¼ä
#define ATOVERTIME4 10000 //ÉèÖÃÆÕÍ¨ATÖ¸ÁîµÄ³¬Ê±Ê±¼ä
#define ATOVERTIME5 20000 //ÉèÖÃÆÕÍ¨ATÖ¸ÁîµÄ³¬Ê±Ê±¼ä
#define ATOVERTIME6 30000 //ÉèÖÃÆÕÍ¨ATÖ¸ÁîµÄ³¬Ê±Ê±¼ä
unsigned char STRC[256]=
{//¸Ã±í²»Çø·Ö×ÖÄ¸´óĞ¡Ğ´
	0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
	16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
	32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
	48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,//A
	64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,//
	80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,
	96,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,
	80,81,82,83,84,85,86,87,88,89,90,123,124,125,126,127,
	128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
	144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
	160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
	176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
	192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,
	208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,
	224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,
	240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255
};
u8 LM2M_VER[200] = {0};

extern u16 MDM_write(u8 *data, u16 datalen);

static _SIMinf Lsiminf;
static _M2Minf Lm2minf;
static _M2Mstatus Lm2mstatus;

void AT_Init(u8 *seraddr, u32 port){
	Lsiminf.tellen = 0;
	Lsiminf.ccidlen = 0;
	Lsiminf.tsp = 0;
	Lsiminf.business = 0;
	Lsiminf.simfun = 0;
	Lsiminf.province = 0;
	Lsiminf.year = 0;
	Lsiminf.supplier = 0;
	
	Lm2minf.imeilen = 0;
	Lm2minf.cimilen = 0;
	Lm2minf.mmc = 0;
	Lm2minf.mnc = 0;
	
	Lm2mstatus.modemstart = 0;
	Lm2mstatus.gprsbond = 0;
	Lm2mstatus.gprslink = 0;
	Lm2mstatus.m2mstatus = 0;
	Lm2mstatus.csq = 0;
	
	memset((char *)LM2M_VER, 0, 200);
	Lm2mstatus.seraddr[0] = 0;
	if(0 == seraddr || 0 == *(seraddr + 0)){//¸¨IPÒì³£Ê±²ÉÓÃÖ÷IPµØÖ·
		user_debug("i:server IP");
		strcpy(Lm2mstatus.seraddr, db_svr_addrget());
	  Lm2mstatus.port = db_svr_portget();
	}
	else{
		user_debug("i:server1 IP[%s,%d]", seraddr, port);
	  if(seraddr != NULL && strlen(seraddr) < 128)strcpy(Lm2mstatus.seraddr, seraddr);
	  Lm2mstatus.port = port;
	}
}

/*ÁÙÊ±ĞŞ¸Ä·şÎñÆ÷µØÖ·
*/
u8 m2m_svr_addrtemplerset(u8 *addr, u32 port){
	if(NULL == addr)return 1;
	user_debug("i:ser-ip-set:%s:%d", addr,port);
	strcpy((s8 *)Lm2mstatus.seraddr, (s8 *)addr);
	Lm2mstatus.port = port;
	
	
	return 0;
}

/*ÅĞ¶ÏÊÇ·ñÎªÍ¬Ò»¸ö·şÎñÆ÷µØÖ·
*/
u8 m2m_ser_addrcheck(u8 *addr){
	u8 len1,len2;
	
	if(NULL == addr)return 1;
	len1 = strlen((s8 *)addr);
	len2 = strlen((s8 *)Lm2mstatus.seraddr);
	if(len1 != len2)return 2;
	for(len2 = 0; len2 < len1; len2 ++){
		if(*(addr + len2) != Lm2mstatus.seraddr[len2])return 3;
	}
	return 0;
}

u8 m2m_svr_addrreback(void){
	strcpy((s8 *)Lm2mstatus.seraddr, (s8 *)db_svr_addrget());
	Lm2mstatus.port = db_svr_portget();
	
	return 0;
}

void AT_delay(u32 time){
	eat_sleep(time);
}
/*
*¿ÉÒÔ·¢ËÍ×Ö·û´®ÒÔ¼°BCDÂëÊı¾İ£¬µ«ĞèÒª¸ø³öÊı¾İ³¤¶È
******************************************/
u16 AT_Send(u8 *data, u16 datalen){
	if(NULL == data || 0 == datalen)return 0;
	return MDM_write(data, datalen);
}
/***
*Ö»ÄÜ·¢ËÍ×Ö·û´®Êı¾İ
***********************/
u16 AT_SendEx(u8 *data){
	u16 datalen;
	if(NULL == data)return 0;
	datalen = strlen(data);
	return MDM_write(data, datalen);
}

/*
*¿ÉÒÔ·¢ËÍ×Ö·û´®ÒÔ¼°BCDÂëÊı¾İ£¬µ«ĞèÒª¸ø³öÊı¾İ³¤¶È
******************************************/
u16 AT_Send1(u8 *data, u16 datalen){
	if(NULL == data || 00 == datalen)return 0;
	return MDM_write1(data, datalen);
}
/***
*Ö»ÄÜ·¢ËÍ×Ö·û´®Êı¾İ
***********************/
u16 AT_Send1Ex(u8 *data){
	u16 datalen;
	if(NULL == data)return 0;
	datalen = strlen(data);
	return MDM_write1(data, datalen);
}
/*ATÖ¸Áî½ÓÊÕ
*¸Ã½Ó¿ÚÓÃÓÚ½ÓÊÕSIMÄ£¿éµÄÊı¾İ·µ»Ø  ²¢¼ì²âÊÇ·ñÓĞÒªµÃÊı¾İ
*Input: time = ³¬Ê±Ê±¼ä ÒÔmsÎªµ¥Î»
	      str1 = Òª²éÕÒµÄÊı¾İ
	      str2 = Òª²éÕÒµÄÊı¾İ
	   Èç¹ûstr1/str2¾ùÎª¿Õ±íÊ¾½ÓÊÕµ½Êı¾İºóÁ¢¼´·µ»Ø
	   str1/str2ÖĞµÄ'?'·ûºÅ±íÊ¾²»ĞèÒªÆ¥Åä
*Return: 0 = ³¬Ê±Î´ÊÕµ½ÈÎºÎÊı¾İ
	       1 = ÊÕµ½°üº¬str1µÄÄÚÈİ
	       2 = ÊÕµ½°üº¬str2µÄÄÚÈİ
	       ff = µ±str1/str2¾ùÎª¿Õ  ½ÓÊÕµ½Êı¾İ
*/
u8 AT_Read(u32 time, u8 *str1, u8 *str2)
{
	u8 *rec;
	u32 t;
	u16 reclen,str1len, str2len, strflag,index;
	
	str1len = 0x00;
	str2len = 0x00;
	if(str1)str1len = strlen(str1);
	if(str2)str2len = strlen(str2);
	
	t = 0;
	while(1)
	{
		reclen = MDM_DataToApp(&rec);
		if(reclen != 0x00)
		{
			//user_debug("AT_Read[%d]:%s]",reclen,rec);
			if('E' == *(rec+0) && 'R' == *(rec+1)&& 'R' == *(rec+2)&& 'O' == *(rec+3)&& 'R' == *(rec+4)){
				return 0x7f;
			}
			if(0x00 == str1len && 0x00 == str2len)
			{
				return 0xff;
			}
			if(str1len)
			{//str1²»Îª¿Õ
				 strflag = 0x00;
				 for(index = 0x00; index < reclen; index ++)
			   {
			   	   if('?' == *(str1 + strflag) || STRC[*(str1 + strflag)] == STRC[*(rec + index)])
			   	   {//?×÷ÎªÍ¨Åä·û
			   	   	   strflag ++;
			   	   	   if(strflag == str1len)
			   	   	   {
			   	   	   	  return 0x01;
			   	   	   }
			   	   }
			   	   else
			   	   {
			   	   	   strflag = 0x00;
			   	   }
			   }
			}
			if(str2len)
			{//str2²»Îª¿Õ
				 strflag = 0x00;
				 for(index = 0x00; index < reclen; index ++)
			   {
			   	   if('?' == *(str2 + strflag) || STRC[*(str2 + strflag)] == STRC[*(rec + index)])
			   	   {//?×÷ÎªÍ¨Åä·û
			   	   	   strflag ++;
			   	   	   if(strflag == str2len)
			   	   	   {
			   	   	   	  return 0x02;
			   	   	   }
			   	   }
			   	   else
			   	   {
			   	   	   strflag = 0x00;
			   	   }
			   }
			}
		}
		t ++;
		if(t * 5  >= time)
		{
			user_debug("i:no data");
			return 0x00;
		}
		eat_sleep(5);
	}
}


/*µ±½ÓÊÕµ½µÄÊı¾İ°üº¬str1ÄÚÈİÊ±·µ»Ø¸ÃÊı¾İ³¤¶È
*/
u16 AT_ReadEx(u32 time, u8 *str1, u8 **dataout)
{
	u8 *rec;
	u32 t;
	u16 reclen,str1len, strflag,index;
	
	str1len = 0x00;
	if(str1)str1len = strlen(str1);
	t = 0;
	while(1)
	{
		
		reclen = MDM_DataToApp(&rec);
		if(reclen != 0x00)
		{
			//user_debug("AT_ReadEx[%d]:%s]",reclen,rec);
			if('E' == *(rec+0) && 'R' == *(rec+1)&& 'R' == *(rec+2)&& 'O' == *(rec+3)&& 'R' == *(rec+4)){
				return 0;
			}
			if(str1len)
			{//str1²»Îª¿Õ
				 strflag = 0x00;
				 for(index = 0x00; index < reclen; index ++)
			   {
			   	   if('?' == *(str1 + strflag) || STRC[*(str1 + strflag)] == STRC[*(rec + index)])
			   	   {//?×÷ÎªÍ¨Åä·û
			   	   	   strflag ++;
			   	   	   if(strflag == str1len)
			   	   	   {
			   	   	   	   *dataout = rec;
			   	   	   	   return reclen;
			   	   	   }
			   	   }
			   	   else
			   	   {
			   	   	   strflag = 0x00;
			   	   }
			   }
			}
			else{
			 *dataout = rec;
			 return reclen;
			}
		}
		t ++;
		if(t * 5 >= time)
		{
			return 0x00;
		}
		eat_sleep(5);
	}
}
/*·µ»Østr1Î»ÖÃÆğµÄÊı¾İ
*/
u16 AT_ReadExEx(u32 time, u8 *str1, u8 **dataout)
{
	u8 *rec;
	u32 t;
	u16 reclen,str1len, strflag,index;
	
	str1len = 0x00;
	if(str1)str1len = strlen(str1);
	t = 0;
	while(1)
	{
		
		reclen = MDM_DataToApp(&rec);
		if(reclen != 0x00)
		{
			//user_debug("AT_ReadEx[%d]:%s]",reclen,rec);
			if('E' == *(rec+0) && 'R' == *(rec+1)&& 'R' == *(rec+2)&& 'O' == *(rec+3)&& 'R' == *(rec+4)){
				return 0;
			}
			if(str1len)
			{//str1²»Îª¿Õ
				 strflag = 0x00;
				 for(index = 0x00; index < reclen; index ++)
			   {
			   	   if('?' == *(str1 + strflag) || STRC[*(str1 + strflag)] == STRC[*(rec + index)])
			   	   {//?×÷ÎªÍ¨Åä·û
			   	   	   strflag ++;
			   	   	   if(strflag == str1len)
			   	   	   {
			   	   	   	   *dataout = rec + (index - strflag);
			   	   	   	   return reclen - index + strflag;
			   	   	   }
			   	   }
			   	   else
			   	   {
			   	   	   strflag = 0x00;
			   	   }
			   }
			}
			else{
			 *dataout = rec;
			 return reclen;
			}
		}
		t ++;
		if(t * 5 >= time)
		{
			return 0x00;
		}
		eat_sleep(5);
	}
}
/*Á¬½ÓSIMÄ£¿é£¬Ö÷ÒªÓÃÀ´¼ì²âSIMÄ£¿éÊÇ·ñ¹¤×÷
*µ¥Æ¬»úÊÇ·ñÓëSIM½øĞĞÁ¬½Ó 
*Ö¸Áî£ºAT
*»ØÓ¦£ºOK  Èç¹ûÓĞ»ØÏÔÔòSIMÊ×ÏÈ»ØÓ¦ AT \r\n \r\n
*Ö»ÒªÔÚ5sÖÓÖ®ÄÚÓĞ»ØÓ¦ÇÒ·ÇerrorÔòÈÏÎªÕı³£
*Ö§³Ö´ø»ØÏÔºÍ²»´ø»ØÏÔ
*½ÓÊÕ¹ı³ÌÓ¦¸ÃÊÇ£º
*    [AT\r\n]
*    \r\n
*    OK\r\n
*Return: 0=Ê§°Ü
	       1=³É¹¦
*/
u8 AT_AT(void)
{
	u8 *AT = "AT";
	u8 index;
	
	for(index = 0x00; index < 2; index ++)
	{
	   AT_SendEx(AT);
	   if(0x01 == AT_Read(ATOVERTIME1,"OK",NULL))return 0x01;
	}
	return 0x00;
}

/*×Ô¶¯½ÓÌıÊ¹ÄÜ
*/
u8 ATS0(void)
{
	u8 *AT = "ATS0=1";
	u8 index;
	
	for(index = 0x00; index < 2; index ++)
	{
	   AT_SendEx(AT);
	   if(0x01 == AT_Read(ATOVERTIME1,"OK",NULL))return 0x01;
	}
	return 0x00;
}


/**
*²éÑ¯SIM¿¨ÊÇ·ñ´æÔÚ£¬modemÆô¶¯ºóÁ¢¿ÌÒª¼ì²âSIM¿¨ÊÇ·ñ´æÔÚ£¬
*Èç¹û²»´æÔÚÔò²»ÄÜÖ´ĞĞºóĞĞ²Ù×÷
*Ö¸Áî£ºAT+CPIN?
*»ØÓ¦£º+CPIN:READY
*       OK
*Return: 0=Ê§°Ü
	       1=³É¹¦
**************************************************/
u8 AT_CPIN(void)
{
	u8 *CPIN = "AT+CPIN?";
	u8 index;
	
	for(index = 0x00; index < 2; index ++)
	{
	   AT_SendEx(CPIN);
	   if(0x01 == AT_Read(500,"OK",NULL))return 0x01;
	}
	return 0x00;
}

/*ÈíÆô¶¯
*Ö¸Áî¡ÃAT+CFUN=1,1
*»ØÓ¦£ºIIII
*      RDY
*      OK
*½ÓÊÕµ½Call Ready±íÊ¾
*/
u8 AT_CFUN(u8 status)
{
	u8 CMD[32];
	u8 index;
	
	sprintf(CMD, "AT+CFUN=%d",status);
	for(index = 0; index < 3; index ++){
	   AT_SendEx(CMD);
	   if(1 == AT_Read(ATOVERTIME1,"OK",NULL))return 1;
	   eat_sleep(500);
  }
  return 0;
}

/*SIM»ù±¾ÉèÖÃ  Ã¿´ÎSIMÆô¶¯£¨Ó²¼ş¡¢Èí¼şÆô¶¯£©ĞèÒª½øĞĞ¸ÃÉèÖÃ
*»ØÏÔÄ£Ê½¹Ø±Õ£ºATE0      
*              OK
*Ö¸Áî½á¹û·µ»Ø£ºATQ0
*              OK
*·µ»Ø½á¹ûÎª×Ö·û·½Ê½£ºATV1
*                    OK
*ATE0:¹Ø»ØÏÔ
*ATQ0:Ã¿ÌõATÖ´ĞĞºó¶¼»áÓĞÏàÓ¦·µ»Ø
*ATV1£ºÃ¿ÌõATÖ´ĞĞºó·µ»ØµÄÊÇ×Ö·ûĞÅÏ¢
*Ö§³Ö´ø»ØÏÔºÍ²»´ø»ØÏÔ
*/
u8 AT_EQV(void)
{
  u8 *CMD[3] = {"ATE0","ATQ0","ATV1"};
	u8 index;
	
	for(index = 0x00; index < 3; index ++)
	{
		AT_SendEx(CMD[index]);
		if(0x00 == AT_Read(ATOVERTIME1,"OK",NULL))return 0x00;
	}
	return 0x01;
}


/*»ñÈ¡ĞÅºÅÖÊÁ¿
*Ö¸Áî£ºAT+CSQ
*»ØÓ¦£º+CSQ: 23,0
*    £ºOK
<RSSI>     0              Ğ¡ÓÚµÈÓÚ-115dBm
                 1               -111dBm
                 2¡«30       -110¡«-54dBm
                 31            ´óÓÚµÈÓÚ-52dBm
                 99            Î´Öª²ÎÊı
*/
u8 AT_CSQ(void)
{
	char *CMD = "AT+CSQ";
	u16 datalen,dataindex;
	u8 *res;
	u8 csq;
	
	csq = 0;
	AT_SendEx(CMD);
	datalen = AT_ReadEx(ATOVERTIME0, "CSQ", &res);
	//user_debug("AT_CSQ:[%d]%s",datalen, res);
	if(datalen > 5 && datalen < 250){
		for(dataindex = 0; dataindex < datalen - 4; dataindex ++){
		   if('C' == *(res + dataindex) && 'S' == *(res + dataindex + 1) &&'Q' == *(res + dataindex + 2)){
		   	dataindex += 3;
		   	while(1){
		   		if(*(res + dataindex) >= '0' && *(res + dataindex) <= '9')break;
		   		dataindex ++;
		   		if(dataindex >= datalen){
		   			user_infor("e:CSQ1:[%d-%d]",dataindex,datalen);
		   			return csq;
		   		}
		   	}
		   	while(1){
		   		if(*(res + dataindex) >= '0' && *(res + dataindex) <= '9'){
		   			csq = csq * 10 + (*(res + dataindex) - '0');
		   			dataindex ++;
		   			if(dataindex >= datalen){
		   				user_infor("e:CSQ2:[%d-%d]",dataindex,csq);
		   				return csq;
		   			}
		   		}
		   		else{
		   			 //user_debug("CSQ3:[%d-%d]",dataindex,csq);
		   			 return csq;
		   		}
		   	}
		  }
		}
	}
	return csq;
}


/*²éÑ¯SIMÊÇ·ñ·û×ÅGPRS
*Ö¸Áî£ºAT+CGATT?                // ²éÑ¯Ä£¿éÊÇ·ñ¸½×ÅGPRSÍøÂç
*»ØÓ¦£º+CGATT: 1       //Èç¹ûÎª0 ÔòĞèÒªÍ¨¹ıAT+CGATT=1½øĞĞÉèÖÃ£¬·ñÔò²»ĞèÒª
*      OK
*¼ì²âSIMÊÇ·ñ·û×ÅGPRS£¬Èç¹û·û×ÅÖ±½Ó·µ»Ø£¬·ñÔòÔòÉèÖÃ·û×Å 
*/
u8 AT_CGATT(void)
{
  u8 *CMD = "AT+CGATT?";
	u8 *CMD1 = "AT+CGATT=1";
	u8 res;
	
	AT_SendEx(CMD);
	res = AT_Read(ATOVERTIME1,"CGATT: 1",NULL); 
	if(0x01 ==  res)
	{
		return 0x01;
	}
	else
	{//Î´×¢²á
		AT_SendEx(CMD1);
	  if(0x01 == AT_Read(ATOVERTIME1,"OK",NULL))return 0x01; 
	}
	return 0x00;
}

/*»ñÈ¡ÍøÂç×¢²áĞÅÏ¢
*Ö¸Áî£ºAT+CREG?
*·µ»Ø£º+CREG: 1,1     Ö÷Òª¿´µÚ¶ş×Ö½Ú£¬Èç¹ûÊÇ1»ò5±íÊ¾ÒÑ¾­×¢²á£¬·ñÔòÎ´×¢²á£¬Èç¹û            
                      Î´×¢²áÔò²ÉÓÃAT+CREG=1½øĞĞ×¢²á
*    £ºOK
*¼ì²âÍøÂçÊÇ·ñ×¢²á£¬Èç¹ûÒÑ¾­×¢²áÖ±½Ó·µ»Ø01 ·µ»Ø½øĞĞ×¢²á²¢¼ì²â½á¹û£¬Èç¹û×¢²áÊ§°ÜÔò·µ»Ø0
*/
u8 AT_CREG(void)
{
	u8 *CMD = "AT+CREG?";
	u8 *CMD1 = "AT+CREG=1";
	u8 res;
	
	AT_SendEx(CMD);
	res = AT_Read(ATOVERTIME1,"CREG: ?,1","CREG: ?,5"); 
	if(0x01 ==  res || 0x02 == res)
	{
		return res;
	}
	else
	{//Î´×¢²á
		AT_SendEx(CMD1);
	  if(0x01 == AT_Read(ATOVERTIME1,"OK",NULL))return 0x01; 
	}
	return 0x00;
}

/*²éÑ¯GPRSÍøÂç×¢²á×´Ì¬
*Ö¸Áî£ºAT+CGREG?
*»ØÓ¦£º+CGREG: 1     //Èç¹ûÎª0ÔòĞèÒªÍ¨¹ıAT+CGREG=1½øĞĞ²âÊÔ£¬·ñÔò²»ĞèÒª
*»ØÓ¦£ºOK
*¼ì²âGPRSÊÇ·ñ×¢²áµ½ÍøÂç  Èç¹ûÒÑ¾­×¢²áÖ±½Ó·µ»Ø  ·ñÔò·µ»Ø½øĞĞ×¢²á
*/
u8 AT_CGREG(void)
{
	u8 *CMD = "AT+CGREG?";
	u8 *CMD1 = "AT+CGREG=1";
	u8 res;
	
	AT_SendEx(CMD);
	res = AT_Read(ATOVERTIME1,"CGREG: ?,1","CGREG: ?,5"); 
	if(0x01 ==  res || 0x02 ==  res)
	{
		return 0x01;
	}
	else
	{//Î´×¢²á
		AT_SendEx(CMD1);
	  if(0x01 == AT_Read(ATOVERTIME1,"OK",NULL))return 0x01; 
	}
	return 0x00;
}

/*ÉèÖÃ¶àTCP/IPÁ´½Ó
*/
u8 AT_CIPMUX(void){
	u8 *CMD = "AT+CIPMUX=1";
	
	AT_SendEx(CMD);
	
	if(0x01 == AT_Read(ATOVERTIME0,"OK",NULL)){
		return 0;
	}
	return 1;
}


/*»ñÈ¡±¾»úºÅÂë  ²»ÊÇËùÓĞSIM¶¼ÄÜ»ñÈ¡
*Ö¸Áî£ºAT+CNUM
*»ØÓ¦£ººÅÂë
*      OK
*Ê§°Ü·µ»Ø0
*³É¹¦·µ»ØºÅÂë³¤¶È 11
*/
u8 AT_CNUM(u8 *num)
{
	u8 *CMD = "AT+CNUM";
	u16 datalen,dataindex,index;
	u8 *res,data[13];
	
	AT_SendEx(CMD);
	datalen = AT_ReadEx(ATOVERTIME1,"CNUM:", &res); 
	if(0x00 ==  datalen)
	{
		return 0x00;
	}
	else
	{//¶Á³öºÅÂë Êı¾İÔÚATDATAÖĞ
		 if(datalen < 20)return 0x00;
		 dataindex = 0x00;
	   for(index = 0x00; index < datalen; index ++)
	   {
	   	 if('"' == *(res + index))
	   	 {
	   	 	  if(dataindex != 0x00 && dataindex <11)
	   	 	  {
	   	 	  	dataindex = 0x00;
	   	 	  }
	   	 	  else if(dataindex != 0x00 && dataindex >= 11)
	   	 	  {
	   	 	  	data[dataindex] = 0x00;
	   	 	  	strcpy((char *)num, (char *)&data[0x01]);
	   	 	  	return 11;
	   	 	  }
	   	 	  data[dataindex] = 't';
	   	 	  dataindex ++;
	   	 }
	   	 else if(dataindex)
	   	 {
	   	 	  data[dataindex] = *(res  + index);
	   	 	  dataindex ++;
	   	 }
	   }
	}
	return 0x00;
}

/*AT+CMGF
*/
/*AT+CNMI ¶ÌĞÅÉèÖÃ  Éè±¸M2M½ÓÊÕµ½¶ÌĞÅºó×Ô¶¯·µ»Ø
*/
u8 AT_CNMI(void){
	u8 *CMGDA = "AT+CNMI=2,2,0,0,0";
	u8 loop;
	
	for(loop = 0; loop < 3; loop ++){
	   AT_SendEx(CMGDA);
	   if(1 == AT_Read(ATOVERTIME1,"OK",NULL))return 1;
	}
	user_debug("i:AT_CNMI error");
	return 0;
}


u8 AT_CMGDA(void){
	u8 *CMGDA = "AT+CMGDA=6";//É¾³ıËùÓĞ¶ÌĞÅ PDU¸ñÊ½
	u8 *CMGDAt = "AT+CMGDA=\"DEL ALL\"";//É¾³ıËùÓĞ¶ÌĞÅ text¸ñÊ½
	
	AT_SendEx(CMGDA);
	AT_Read(ATOVERTIME1,NULL,NULL);
	AT_SendEx(CMGDAt);
	AT_Read(ATOVERTIME1,NULL,NULL);
	return 1;
}

u8 AT_SMSinit(void){
	static u8 flag = 0;
	u8 *CMGF = "AT+CMGF=1";
  u8 *CSCS = "AT+CSCS=\"GSM\"";
  u8 *CSMP = "AT+CSMP=17,167,0,241";
  s8 *CNMI = "AT+CNMI=2,2,0,0,0";
  u8 result,loop;
  
  if(0x0f == (flag & 0x0f))return 0;
  result = 0;
  if((flag & 0x01) == 0){
      for(loop = 0; loop  < 1; loop ++){
         AT_SendEx(CMGF);
         if(1 == AT_Read(ATOVERTIME0,"OK",NULL)){
         	flag |= 0x01;
         	break;
         }
         eat_sleep(100);
      }
  }
  if(0 == (flag & 0x02)){
      for(loop = 0; loop < 1; loop ++){
         AT_SendEx(CSCS);
         if(1 == AT_Read(ATOVERTIME0,"OK",NULL)){
         	flag |= 0x02;
         	break;
         }
         eat_sleep(100);
      }
  }
  if(0 == (flag & 0x04)){
      for(loop = 0; loop < 1; loop ++){
         AT_SendEx(CSMP);
         if(1 == AT_Read(ATOVERTIME0,"OK",NULL)){
         	flag |= 0x04;
         	break;
        }
         eat_sleep(100);
      }
  }
  if(0 == (flag & 0x08)){
      for(loop = 0; loop < 1; loop ++){
         AT_SendEx(CNMI);
         if(1 == AT_Read(ATOVERTIME0,"OK",NULL)){
         	flag |= 0x08;
         	break;
        }
        eat_sleep(100);
      }
  }
  user_debug("i:AT_SMSinit=%02x", flag);
  return flag;
}
/*********¶ÌĞÅ·¢ËÍ
*·¢ËÍÓ¢ÎÄ¶ÌĞÅ
*AT+CMGS=¡°1372876114¡±
***********************/
u8 AT_SMSENDex(u8 *tell, u8 *data)
{
 u8 send[64];
 u8 *tellex;
 u8 *SMsend="AT+CMGS=";
 
 if(NULL == data)return 1;
 if(NULL == tell)tellex = db_svr_ttellget();
 else tellex = tell;
 if(NULL == tellex)return 1;
 
 user_debug("i:SMS:%s", data);
 strcpy(send, SMsend);
 //strcat(send,"\"");
// strcat(send,"13728716114");
 //strcat(send,"\"");
 
 strcat((s8 *)send,"\"");
 strcat((s8 *)send,(s8 *)tellex);
 strcat((s8 *)send,"\"");
 
 AT_SendEx(send);
 if(0xff == AT_Read(ATOVERTIME1,NULL,NULL));//Ö»ÒªÊÕµ½·µ»ØÁ¢¿Ì·¢ËÍ
 else
 {
  return 0x01;
 }
 AT_SendEx(data);
 send[0] = 0x1a;
 send[1] = 0;
 AT_Send1(send, 1);
 return 0x00;
}

/*»ñÈ¡IMEI
*Ö¸Áî£ºAT+GSN
*»ØÓ¦£ºIMEI
*      OK
*OUTPUT: IMEI --- ´æ·ÅIMEI
*RETURN: 0 =Ê§°Ü
	       ·Ç0 = IMEI×Ö½Ú¸öÊı  ±ØĞëÎª15
*/
u8 AT_GSN(u8 *IMEI)
{
	u8 *CMD = "AT+GSN";
	u16 datalen, index,index1,loopn;
	u8 *res;
	
	if(0x00 == IMEI)
	{
		return 0x00;
	}
	AT_SendEx(CMD);
	for(loopn = 0x00; loopn < 3; loopn ++)
	{
		  datalen = AT_ReadEx(ATOVERTIME1,NULL,&res);
	    if(datalen >= 15)
	    {
	        index1 = 0x00;
	        for(index = 0x00; index < datalen; index ++)
	        {
	        	if(*(res + index) != 0x0a && *(res + index) != 0x0d)
	        	{
	        		*(IMEI + index1) = *(res + index);
	        		index1 ++;
	        		if(15 == index1)
	        		{
	        			return index1;
	        		}
	        	}
	        	else
	        	{
	        		index1 = 0x00;
	        	}
	        }
	    }
  }
	return 0x00;
}


/*»ñÈ¡CIMI
*Ö¸Áî£ºAT+CIMI
*»ØÓ¦£ºCIMI
       OK
*OUTPUT: CIMI --- CIMI
*RETURN: 0 =Ê§°Ü
	       ·Ç0 = CIMI×Ö½Ú¸öÊı  ±ØĞëÎª15
*/
u8 AT_CIMI(u8 *CIMI)
{
	u8 *CMD = "AT+CIMI";
	u16 datalen, index,index1,loopn;
	u8 *res;
	
	if(0x00 == CIMI)
	{
		return 0x00;
	}
	AT_SendEx(CMD);
	for(loopn = 0x00; loopn < 3; loopn ++)
	{
		  datalen = AT_ReadEx(ATOVERTIME1,NULL,&res);
	    if(datalen >= 15)
	    {
	        index1 = 0x00;
	        for(index = 0x00; index < datalen; index ++)
	        {
	        	if(*(res + index) != 0x0a && *(res + index) != 0x0d)
	        	{
	        		*(CIMI + index1) = *(res + index);
	        		index1 ++;
	        		if(15 == index1)
	        		{
	        			return index1;
	        		}
	        	}
	        	else
	        	{
	        		index1 = 0x00;
	        	}
	        }
	    }
	}
	return 0x00;
}

//À´µç½ÓÌı
void AT_ATA(void){
	u8 *ATA= "ATA";
	
	AT_SendEx(ATA);
}
//À´µç¹Ò¶Ï
void AT_ATH(void){
	u8 *ATH = "ATH";
	
	AT_SendEx(ATH);
}


/*µçÑ¹¼ì²â ·µ»Øµç³ØµçÑ¹
*/
static u32 Lvol = 0;
u32 m2m_volget(void){
	return Lvol;
}
u32 AT_CBC(void){
	char *CBC = "AT+CBC";
	u32 vol;
	u8 *dataout;
	u8 res;
	u8 loop,u8t1,u8t2;
	
	for(loop = 0; loop < 2; loop++){
	    AT_SendEx(CBC);
	    dataout = NULL;
	    res = AT_ReadEx(ATOVERTIME0,"CBC",&dataout);
	    if(res && dataout != NULL){
	    	//user_debug("CBC:%s",dataout);
	    	for(u8t1 = 0; u8t1 < strlen(dataout); u8t1 ++){
	    		if(',' == *(dataout + u8t1)){
	    			u8t1 ++;
	    			for(; u8t1 < strlen(dataout); u8t1 ++){
	    				if(',' == *(dataout + u8t1)){
	    					u8t1 ++;
	    					vol = 0;
	    					for(; u8t1 < strlen(dataout); u8t1 ++){
	    						if(0x0d == *(dataout + u8t1) || 0x0a == *(dataout + u8t1)){
	    							Lvol = vol;
	    							return vol;
	    						}
	    						vol = vol * 10 + (*(dataout + u8t1) - '0');
	    					}
	    					Lvol = vol;
	    					return vol;
	    				}
	    			}
	    		}
	    	}
	    }
  }
  return 0xffff;
}


/*ADC¼ì²â Í¨¹ı¸Ã¼ì²â»ñÈ¡Éè±¸ÊÇ·ñÒÑ¾­°Î³ö
*·µ»Ø: 0=Õı³£
*      1=µçÑ¹Òì³£ ¿ÉÄÜ±»°Î³ö
*Á¬Ğø3´Î¼ì²âÈç¹ûµçÑ¹¾ùĞ¡ÓÚ1VÔò±íÊ¾Òì³£ÍË³ö
*±¨¾¯Ô­Ôò: ¼ì²âµ½µÍµçÆ½ºó±¨¾¯,²»Á¬Ğø±¨¾¯,±¨¾¯ºóÖ»ÓĞµÈÔÙ´Î¼ì²âµ½Õı³£µçÑ¹ºó²ÅÄÜÔÙ´Î±¨¾¯
*/
static u8 CADC_WARNING = 0;
u8 AT_CADC(void){
	s8 *AT = "AT+CADC?";
	u8 index,*res,u8t1;
	u16 datalen;
	u32 vol;



	
	for(index = 0x00; index < 3; index ++)
	{
	   AT_SendEx(AT);
	   datalen = AT_ReadEx(ATOVERTIME0,"CADC:", &res); 
	   if(0 == datalen)return 0;
	   //user_debug("CADC:%s", res);
	   for(u8t1 = 0; u8t1 < datalen; u8t1 ++){
	   	if(',' == *(res + u8t1) && '1' == *(res + u8t1 - 1)){
	   		vol = 0;
	   		for(; u8t1 < datalen; u8t1 ++){
	   			if(*(res + u8t1) >= '0' && *(res + u8t1) <= '9'){
	   				vol = vol * 10 + (*(res + u8t1) - '0');
	   			}
	   		}
		
	   		if((vol > 1000)&&(vol!=2800)){
	   			CADC_WARNING = 0;
	   			return 0;
	   		}
	   	}
	  }
	  eat_sleep(300);
	}
	if(CADC_WARNING != 0x55)
	{
		CADC_WARNING = 0x55;
		obd_vol_offenable();//2015/12/17 13:35 fangcuisong
		return 0x01;
	}
	else return 0x00;
}


/*»ñÈ¡CCID  SIMÉÏµÄID
*Ö¸Áî£ºAT+CCID
*»ØÓ¦£ºCIMI
*      OK
*OUTPUT: CCID --- CCID
*RETURN: 0 =Ê§°Ü
	       ·Ç0 = CCID×Ö½Ú¸öÊı  ±ØĞëÎª20
*/
u8 AT_CCID(u8 *CCID)
{
	u8 *CMD = "AT+CCID";
	u16 datalen, index,index1,loopn;
	u8 *res;
	
	if(0x00 == CCID)
	{
		return 0x00;
	}
	AT_SendEx(CMD);
	for(loopn = 0x00; loopn < 3; loopn ++)
	{
		  datalen = AT_ReadEx(ATOVERTIME1,NULL,&res);
	    if(datalen >= 20)
	    {
	        index1 = 0x00;
	        for(index = 0x00; index < datalen; index ++)
	        {
	        	if(*(res + index) != 0x0a && *(res + index) != 0x0d)
	        	{
	        		*(CCID + index1) = *(res + index);
	        		index1 ++;
	        		if(20 == index1)
	        		{
	        			return index1;
	        		}
	        	}
	        	else
	        	{
	        		index1 = 0x00;
	        	}
	        }
	    }
	}
	return 0x00;
}
u8 *m2m_ccidget(void){
	return (u8 *)&Lsiminf.ccid[0];
}
void m2m_ccidread(void){
	
	if(Lsiminf.ccid[0] != 0)return;
	AT_CCID(Lsiminf.ccid);
}


/*¼¤»îÒÆ¶¯ÏÖ³¡
*Ö¸Áî£ºAT+CIICR     ³¬Ê±ÉèÖÃ40s
*·µ»Ø£ºOK
*/
u8 AT_CIICR(void)
{
	char *CMD = "AT+CIICR";
	unsigned char res;
	
	AT_SendEx(CMD);
	res = AT_Read(ATOVERTIME3,"OK",NULL);
	if(0x01 == res)return 0x01;
	else
	{
		return 0x00;
	}
}


/*ÉùÒôÊä³öÉèÖÃ Í¨µÀÑ¡Ôñ AT+CSDVC=3  AT+CHFA=1  at+CLVL
*/
u8 AT_CHFA(void){
	char *CMD = "AT+CHFA=1";
	u8 index;//chfa
	
	for(index = 0x00; index < 2; index ++)
	{
	   AT_SendEx(CMD);
	   if(0x01 == AT_Read(ATOVERTIME0,"OK",NULL))return 0x01;
	}
	return 0x00;
}

/*ÒôÁ¿ÉèÖÃ
*/
u8 AT_CLVL(u8 leve){
	s8 CMD[32];
	u8 index;//chfa
	
	sprintf(CMD, "AT+CLVL=%d", leve);
	for(index = 0x00; index < 2; index ++)
	{
	   AT_SendEx(CMD);
	   if(0x01 == AT_Read(ATOVERTIME0,"OK",NULL))return 0x01;
	}
	return 0x00;
}

/*»ñÈ¡±¾»úIP 
*Ö¸Áî£ºAT+CIFSR
*·µ»Ø£ºIP
*    £ºOK
*\CR\LF10.91.107.16\CR\L
*/
u8 AT_CIFSR(u8 *ip)
{
	u8 *CMD = "AT+CIFSR";
	u8 datalen,dataindex,index,loopn;
	u8 *res;
	u8 data[25];
	
	AT_SendEx(CMD);
	for(loopn = 0x00; loopn < 2; loopn ++)
	{
		  datalen = AT_ReadEx(ATOVERTIME2,NULL,&res);
	    if(datalen >= 7)
	    {
	    	dataindex = 0x00;
	      for(index = 0x00; index < datalen; index ++)
	      {
	   	     if(*(res + index) == 0x0a || *(res + index) == 0x0d);//¹ıÂËµô»Ø³µ»»ĞĞ
	   	     else
	   	     {
	   	     	  data[dataindex] = *(res + index);
	   	     	  dataindex ++;
	   	     	  if(dataindex >= 24)break;
	   	     }
	      }
	      data[dataindex] = 0x00;
	      strcpy((char *)ip, (char *)&data[0x00]);
	      return dataindex;
	    }
	}
	return 0x00;
}


/*ÉèÖÃAPN
*Ö¸Áî£ºAT+CSTT=¡±cmnet¡±  ÒÆ¶¯cmnet¡¢cmwap(Ö÷ÒªÊÇ²ÊĞÅÉèÖÃwap) 
*                           ÁªÍ¨uninet¡¢3gnet
*»ØÓ¦£ºOK
*INPUT: flag = 0:ÒÆ¶¯
	             1:Á¬Í¨
	             2£ºÁ¬Í¨3G
³É¹¦·µ»Ø1
Ê§°Ü·µ»Ø0
*/
u8 AT_CSTT(u8 flag)
{
	//u8 *CMD[3] = {"AT+CSTT=\"cmnet\"","AT+CSTT=\"uninet\"","AT+CSTT=\"3gnet\""};
	s8 CMD[64];
	s8 *cmdc[3] = {"cmnet", "uninet", "3gnet"};
	u8 *apn;
	u8 index;
	u8 res;
	
	
	if(flag > 2)return 0x00;
	apn = db_svr_apnget();
	if(NULL == apn || 0 == *(apn)){
		sprintf((s8 *)CMD, "AT+CSTT=\"%s\"", cmdc[flag]);
	}
	else sprintf((s8 *)CMD, "AT+CSTT=\"%s\"", (s8 *)apn);
	
	user_debug("i:AT_CSTT:%d,%s",flag, CMD);
	for(index = 0x00; index < 2; index ++)
	{
	   AT_SendEx(CMD);
	   res = AT_Read(ATOVERTIME1,"OK",NULL);
	   if(0x01 == res)return 0x01;
	   AT_delay(ATOVERTIME2);
  } 
  return 0x00;
}


/*·şÎñÆ÷IPÒÔ¼°¶Ë¿ÚÉèÖÃ
*Ö¸Áî£ºAT+CIPSTART=¡°TCP¡±,¡°116.236.221.75¡±,7015
*·µ»Ø£ºconnect ok
*Ö´ĞĞ¸Ã½Ó¿ÚÇ°±ØĞëÖ´ĞĞsimAPNset(),  simCIICRset(), simIPget()
*/
void AT_TTS(char *mus){
	u8 CMD[136];
	
	if(NULL == mus)return;
	if(strlen(mus) >= 128)return;
	sprintf((char *)CMD, "AT+CTTS=2,\"%s\"", mus);
	AT_SendEx(CMD);
}

/*.wavÎÄ¼ş²¥·Å
*/
void AT_CREC(char *file, u8 mu){
	u8 CMD[136];
	
	if(NULL == file)return;
	if(strlen(file) >= 128)return;
	sprintf((char *)CMD, "AT+CREC=4,\"%s\",1,%d", file, mu);
	AT_SendEx(CMD);
}

/*²âÊÔ¶Ë¿Ú ÓÃCIPSTART5
*/
u8 AT_CIPSTART_test(u8 *ip, u32 port){
	u8 CMD[64];
	u8 data[16];
	
	if(0x00 == ip)
	{
		user_debug("i:IP IS NULL");
		return 0x00;
	}
	strcpy((char *)&CMD[0x00],"AT+CIPSTART=5,");
	strcat((char *)&CMD[0x00],"\"TCP\",\"");
	strcat((char *)&CMD[0x00],ip);
	strcat((char *)&CMD[0x00],"\",");
	sprintf((char *)&data[0x00],"%d",port);
	strcat((char *)&CMD[0x00],(char *)&data[0x00]);
	AT_SendEx(CMD);
	user_debug(CMD);
	if(0x01 == AT_Read(ATOVERTIME3,"CONNECT OK",NULL))return 0x01;
	else
	{
		return 0x00;
	} 
}

u8 AT_CIPSEND_test(u8 *data, u16 datalen)
{
	u8 CMD[32];//="AT+CIPSEND=%d";
	u8 u8result, u8t1;
	u8 loop;
	
	if(NULL == data || 0 == datalen)return 0;
	
  loop = 0;
_loop:
  sprintf((s8 *)CMD, "AT+CIPSEND=5,%d", datalen);
	AT_SendEx(CMD);
	u8result = AT_Read(ATOVERTIME2,">",NULL);
	if(0x01 == u8result);//Ö»ÒªÊÕµ½·µ»ØÁ¢¿Ì·¢ËÍ
	else if(0x7f == u8result){//·µ»Øerror
		  if(loop <3){
		  	eat_sleep(100);
		  	loop ++;
		  	goto _loop;
		  }
	    m2m_statusSet(3);
	    user_debug("i:Restart:error");
	    return 0x7f;
	}
	else
	{
		return 0x02;
	}
	AT_Send1(data, datalen);
	
	eat_sleep(100);//ĞİÃß100ms
	return 0x00;
}

/*·şÎñÆ÷IPÒÔ¼°¶Ë¿ÚÉèÖÃ
*Ö¸Áî£ºAT+CIPSTART=¡°TCP¡±,"  ¡±,7015
*·µ»Ø£ºconnect ok
*Ö´ĞĞ¸Ã½Ó¿ÚÇ°±ØĞëÖ´ĞĞsimAPNset(),  simCIICRset(), simIPget()
*/
u8 AT_CIPSTART(u8 *ip, u32 port)
{
	u8 CMD[64];
	u8 data[16];
	
	if(0x00 == ip)
	{
		user_debug("i:IP IS NULL");
		return 0x00;
	}
	user_debug("lilei-make socket\r\n");
	strcpy((char *)&CMD[0x00],"AT+CIPSTART=0,");
	strcat((char *)&CMD[0x00],"\"TCP\",\"");
	strcat((char *)&CMD[0x00],ip);
	strcat((char *)&CMD[0x00],"\",");
	sprintf((char *)&data[0x00],"%d",port);
	strcat((char *)&CMD[0x00],(char *)&data[0x00]);
	AT_SendEx(CMD);
	user_debug(CMD);
	if(0x01 == AT_Read(ATOVERTIME3,"CONNECT OK",NULL))return 0x01;
	else
	{
		return 0x00;
	} 
	
}

/*·şÎñÆ÷IPÒÔ¼°¶Ë¿ÚÉèÖÃ  ÓÃÓÚÆô¶¯GPSÁ¬½Ó
*Ö¸Áî£ºAT+CIPSTART=¡°TCP¡±,¡°116.236.221.75¡±,7015
*·µ»Ø£ºconnect ok
*Ö´ĞĞ¸Ã½Ó¿ÚÇ°±ØĞëÖ´ĞĞsimAPNset(),  simCIICRset(), simIPget()
*/
u8 AT_CIPSTART_1(u8 *ip, u32 port)
{
	u8 CMD[64];
	u8 data[16];
	
	if(0x00 == ip)
	{
		user_debug("i:IP IS NULL");
		return 0x00;
	}
	strcpy((char *)&CMD[0x00],"AT+CIPSTART=1,");
	strcat((char *)&CMD[0x00],"\"TCP\",\"");
	strcat((char *)&CMD[0x00],ip);
	strcat((char *)&CMD[0x00],"\",");
	sprintf((char *)&data[0x00],"%d",port);
	strcat((char *)&CMD[0x00],(char *)&data[0x00]);
	AT_SendEx(CMD);
	user_debug(CMD);
	if(0x01 == AT_Read(ATOVERTIME3,"CONNECT OK",NULL)){
		return 0x01;
	}
	else
	{
		return 0x00;
	} 
	
}


/*¹Ø±ÕÒÆ¶¯³¡¾°
*Ö¸Áî£ºAT+CIPSHUT
*·µ»Ø£º
*µ±ÍøÂçÁ´Â·±»¶Ï¿ªºóĞèÒªÏÈÓÃ¸ÃÖ¸Áî¹ØÒÆ¶¯³¡¾° 10sÔÙÖØĞÂ½¨Á¢Á¬½Ó
*/
u8 AT_CIPSHUT(void)
{
	char *CMD="AT+CIPSHUT";
	
	AT_SendEx(CMD);
	if(0x01 == AT_Read(ATOVERTIME6,"OK",NULL))return 0x01;
	return 0x00;
}

/*¶Ï¿ªÍøÂç
*Ö¸Áî£ºAT+CIPCLOSE=1    //Îª1±íÊ¾¿ìËÙ¹Ø±Õ100ms£¬Îª0Âı¹Ø 2·ÖÖÓ×óÓÒ
*·µ»Ø£ºOK
Ä£¿éÉèÖÃ³¬Ê±20s£¬Ò»°ãTCP/IPÁ¬½ÓÒì³£Ê¹ÓÃAT+CIPSHUTºó£¬ÖØĞÂÖ´ĞĞAT+CSTTµÈÖØĞÂ½¨Á¢TCP/IPÁ¬½Ó£¬½¨Òé¼ä¸ôÊ±¼ä10s£¬·ñÔò¿ÉÄÜ»áÒòÎªÒÆ¶¯³¡¾°¹Ø±Õ»¹Ã»ÓĞ³¹µ×ÊÍ·Å£¬µ¼ÖÂÖØĞÂ¼¤»îÒÆ¶¯³¡¾°Ê§°Ü¡£
*/
u8 AT_CIPCLOSE(void)
{
	char *CMD="AT+CIPCLOSE=0,1";
	
	AT_SendEx(CMD);
	if(0x01 == AT_Read(ATOVERTIME5,"OK",NULL))return 0x01;
	return 0x00;
}

/*¶Ï¿ªÍøÂç ¶Ï¿ªÓëGPSĞÇÀú·şÎñÆ÷µÄÁ¬½Ó
*Ö¸Áî£ºAT+CIPCLOSE=1    //Îª1±íÊ¾¿ìËÙ¹Ø±Õ100ms£¬Îª0Âı¹Ø 2·ÖÖÓ×óÓÒ
*·µ»Ø£ºOK
Ä£¿éÉèÖÃ³¬Ê±20s£¬Ò»°ãTCP/IPÁ¬½ÓÒì³£Ê¹ÓÃAT+CIPSHUTºó£¬ÖØĞÂÖ´ĞĞAT+CSTTµÈÖØĞÂ½¨Á¢TCP/IPÁ¬½Ó£¬½¨Òé¼ä¸ôÊ±¼ä10s£¬·ñÔò¿ÉÄÜ»áÒòÎªÒÆ¶¯³¡¾°¹Ø±Õ»¹Ã»ÓĞ³¹µ×ÊÍ·Å£¬µ¼ÖÂÖØĞÂ¼¤»îÒÆ¶¯³¡¾°Ê§°Ü¡£
*/
u8 AT_CIPCLOSE_1(void)
{
	char *CMD="AT+CIPCLOSE=1,1";
	
	AT_SendEx(CMD);
	if(0x01 == AT_Read(ATOVERTIME5,"OK",NULL))return 0x01;
	return 0x00;
}


u8 *m2m_verget(void){
	//user_debug("---[%s]", (char *)LM2M_VER);
	return (u8 *)&LM2M_VER[0];
}
/*»ñÈ¡¹Ì¼ş°æ±¾ĞÅÏ¢
*/
u32 AT_VER(void){
	char *CMD = "AT+GSV";
	u32 datalen;
	u8 *res;
	
	AT_SendEx(CMD);
	res = 0;
	datalen = AT_ReadEx(ATOVERTIME1,NULL,&res);
	if(datalen > 5){
	    if(datalen > 180){
	    	datalen = 180;
	    	strncpy((char *)LM2M_VER, (char *)res, datalen);
	    }
	    else strcpy((char *)LM2M_VER, (char *)res);
	    user_debug("i:M2M_VER=%s\r\n",LM2M_VER);                   //add by lilei-2016-04-07
  }
	else datalen = 0;
	return datalen;
}

u8 AT_CIPCLOSE_test(void)
{
	char *CMD="AT+CIPCLOSE=5,1";
	
	AT_SendEx(CMD);
	return 0x01;
}

/*Êı¾İ·¢ËÍ
*Ö¸Áî£ºAT+CIPSEND
*»ØÓ¦£º>                //ÊäÈëÒª·¢ËÍµÄÊı¾İ  1A½áÎ²
*    £ºsend OK
*/
u8 AT_CIPSEND(u8 *data, u16 datalen)
{
	u8 CMD[32];//="AT+CIPSEND=%d";
	u8 u8result, u8t1;
	u8 loop;
	
	if(NULL == data || 0 == datalen)return 0;
	
	/*u8t1 = AT_CSQ();
	if(u8t1 < 4 ){//ĞÅºÅÌ«Èõ
		user_debug("AT_CIPSEND:CSQ low:%d", u8t1);
		return 0x01;
	}*/
	
  loop = 0;
   user_debug("\r\ni:CIPSEND-ToSVR-CMD:%02X,CMD=%02X,Len=%d\r\n",*(data + 8),*(data + 9),datalen);
_loop:
  sprintf((s8 *)CMD, "AT+CIPSEND=0,%d", datalen);
	AT_SendEx(CMD);
	u8result = AT_Read(ATOVERTIME2,">",NULL);
	if(0x01 == u8result);//Ö»ÒªÊÕµ½·µ»ØÁ¢¿Ì·¢ËÍ
	else if(0x7f == u8result){//·µ»Øerror
		  if(loop <3){
		  	eat_sleep(100);
		  	loop ++;
		  	goto _loop;
		  }
	    m2m_statusSet(3);
	    user_debug("i:Restart:error");
	    return 0x7f;
	}
	else
	{
		return 0x02;
	}
	AT_Send1(data, datalen);
	
	eat_sleep(100);//ĞİÃß100ms
	return 0x00;
}


/*Êı¾İ·¢ËÍ ÓÃÓÚ·¢ËÍGPSĞÇÀúÊı¾İ
*Ö¸Áî£ºAT+CIPSEND
*»ØÓ¦£º>                //ÊäÈëÒª·¢ËÍµÄÊı¾İ  1A½áÎ²
*    £ºsend OK
*/
u8 AT_CIPSEND_1(u8 *data, u16 datalen)
{
	u8 CMD[32];//="AT+CIPSEND=%d";
	u8 u8result, u8t1;
	u8 loop;
	
	if(NULL == data || 0 == datalen)return 0;
	
  loop = 0;
_loop:
  sprintf((s8 *)CMD, "AT+CIPSEND=1,%d", datalen);
	AT_SendEx(CMD);
	u8result = AT_Read(ATOVERTIME2,">",NULL);
	if(0x01 == u8result);//Ö»ÒªÊÕµ½·µ»ØÁ¢¿Ì·¢ËÍ
	else if(0x7f == u8result){//·µ»Øerror
		  if(loop <3){
		  	eat_sleep(100);
		  	loop ++;
		  	goto _loop;
		  }
	    user_debug("i:Restart:error");
	    return 0x7f;
	}
	else
	{
		return 0x02;
	}
	AT_Send1(data, datalen);
	
	eat_sleep(100);//ĞİÃß100ms
	return 0x00;
}


/*»ñÈ¡»ùÕ¾ĞÅÏ¢
*Ö¸Áî£ºAT+CENG¿ªÆô»ò¹Ø±Õ¹¤³ÌÄ£Ê½
*·µ»Ø£º+CENG:0,...
*
***************************************************************/
u8 AT_CENG(void)
{
	u8 CMD1[32] ="AT+CENG=1";
	u8 CMD[32] ="AT+CENG?";
	u16 datalen;
	u8 *res;
	
	user_infor("e:AT_CENG");
	AT_SendEx(CMD1);
	if(0x00 == AT_Read(ATOVERTIME1,"OK",NULL))return 0x01;
	
	AT_SendEx(CMD);
	datalen = AT_ReadExEx(ATOVERTIME1,"CENG: 0",&res);
	if(datalen){//½«½ÓÊÕµ½µÄÊı¾İ·¢ËÍµ½GPS»º³åÇø
		gps_mobileinfor(res);
		return 0;
	}
	return 0x03;
}

void AT_CENG_CELL(u32 *lac, u32 *cell){
	u8 CMD[32] ="AT+CENG?";
	u16 datalen, dataindex;
	u8 *res,flag;
	u32 u32lac, u32cell;
	
	if(NULL == lac || NULL == cell)return;
	AT_SendEx(CMD);
	datalen = AT_ReadExEx(ATOVERTIME1,"CENG:",&res);
	u32lac = 0;
	u32cell = 0;
	if(datalen){
		for(dataindex = 0; dataindex < datalen; dataindex ++){
			if('E' == *(res + dataindex) && 'N' == *(res + dataindex +1) && 'G' == *(res + dataindex +2) && ':' == *(res + dataindex+3) && '0' == *(res + dataindex+5)){
				dataindex += 8;
				flag = 0;
				for(; dataindex < datalen; dataindex ++){
					if(0x0d == *(res + dataindex) || 0x0a == *(res + dataindex))break;
					if(',' == *(res + dataindex))flag ++;
					else if(6 == flag){//cellid
						if(*(res + dataindex) >= '0' && *(res + dataindex) <= '9')u32cell = (u32cell << 4) + (*(res + dataindex) - '0');
						else if(*(res + dataindex) >= 'a' && *(res + dataindex) <= 'f')u32cell = (u32cell << 4) + (*(res + dataindex) - 'a') + 0x0a;
						else if(*(res + dataindex) >= 'A' && *(res + dataindex) <= 'F')u32cell = (u32cell << 4) + (*(res + dataindex) - 'A') + 0x0a;
					}
					else if(9 == flag){//lac id
						if(*(res + dataindex) >= '0' && *(res + dataindex) <= '9')u32lac = (u32lac << 4) + (*(res + dataindex) - '0');
						else if(*(res + dataindex) >= 'a' && *(res + dataindex) <= 'f')u32lac = (u32lac << 4) + (*(res + dataindex) - 'a') + 0x0a;
						else if(*(res + dataindex) >= 'A' && *(res + dataindex) <= 'F')u32lac = (u32lac << 4) + (*(res + dataindex) - 'A') + 0x0a;
					}
					if(flag > 9)break;
				}
				break;
			}
		}
	}
	*lac = u32lac;
	*cell = u32cell;
}

/******************************************************************
*UDPÍ¨Ñ¶ ×¨ÎÆ½Ì¨Ğ­ÒéÉè¼Æ
*2015/5/7 21:18
********************************************************************************/
unsigned char UDP_Creat(void){
	//char *cip = "AT+CIPSTART=4,\"UDP\",\"\",30005";//4,%d,\"\",30005
	char cip[64];
	char *cipclose = "AT+CIPCLOSE=4,1";
	unsigned char u8result;//db_svr_addrget
	
	//2015/9/28 10:50 fangcuisong
	sprintf(cip, "AT+CIPSTART=4,\"UDP\",\"%s\",%d", db_svr_addrget(), db_svr_portget());
	AT_SendEx(cip);
	u8result = AT_Read(ATOVERTIME1,"OK","ERROR:");
	if(1 == u8result)return 0;
	else{
	  	AT_SendEx(cip);
	  	u8result = AT_Read(ATOVERTIME1,"OK","ERROR:");
	  	if(1 == u8result)return 0;
	  	else{
	  		 AT_SendEx(cipclose);
	  		 return 1;
	  	}
	}
	return 0;
}

void UDP_Cipclose(void){
	char *cipclose = "AT+CIPCLOSE=4,1";
	
	AT_SendEx(cipclose);
	AT_Read(ATOVERTIME1,"OK","ERROR:");
}

unsigned char UDP_Send(unsigned char *send, int sendlen){
	char CMD[32];
	unsigned char u8result;
	
	if(NULL == send || 0 == sendlen)return 1;
	sprintf(CMD, "AT+CIPSEND=4,%d", sendlen);
	AT_SendEx(CMD);
	u8result = AT_Read(ATOVERTIME1,">","ERROR:");
	if(2 == u8result || 0x7f == u8result){
		return 2;
	}
	AT_Send1(send, sendlen);
	
	return 0;
}

/***
*¼ì²âmodemÊÇ·ñÆô¶¯Õı³£
*Èç¹ûÒÑÆô¶¯ÔòÉèÖÃmodemstart
************************************************/
void m2m_startcheck(void){
	u8 u8result;
	
	user_infor("e:m2m_startcheck >>");
	if(0 == AT_AT())
	{
		Lm2mstatus.modemstart = 0;
		user_debug("i:m2m_startcheck:AT error");
		return;
	}
	
	if(0 == ATS0()){
		Lm2mstatus.modemstart = 0;
		user_debug("m2m_startcheck:ATS0 error");
		return;
	}																					//add by lilei-2017-0623															
	//²ÎÊıÉèÖÃ
	if(0 == AT_EQV())
	{
		Lm2mstatus.modemstart = 0;
		user_debug("i:m2m_startcheck:EQV error");
		return;
	}
	
	AT_CHFA();
	eat_sleep(200);
	AT_CLVL(99);
	
	Lm2mstatus.modemstart = 0x01;
	Lm2mstatus.m2mstatus = 1;
	user_debug("i:m2m_startcheck OK");
	AT_delay(100);
}

/*
*¶ÁÈ¡SIMĞÅÏ¢
**********************************************/
void sim_information(void){
	u8 u8t1;
	
	AT_CBC();
	user_infor("e:sim_information >>");
	if(0 == AT_CPIN())
	{
		user_debug("i:m2m_startcheck:CPIN error");
		return;
	}
  	memset(Lm2minf.cimi, 0, 16);
	u8t1 = AT_CIMI(Lm2minf.cimi);
	if(u8t1)
	{
		if(u8t1 > 15)Lm2minf.cimilen = 15;
		else Lm2minf.cimilen = u8t1;
		if(u8t1 >= 3)
		{
			Lm2minf.mmc = Lm2minf.cimi[0] - '0';
			Lm2minf.mmc = (Lm2minf.mmc << 4) + (Lm2minf.cimi[1] - '0');
			Lm2minf.mmc = (Lm2minf.mmc << 4) + (Lm2minf.cimi[2] - '0');
		}
		if(u8t1 >= 5)
		{
			Lm2minf.mnc = Lm2minf.cimi[3] - '0';
			Lm2minf.mnc = (Lm2minf.mnc << 4) + (Lm2minf.cimi[4] - '0');
		}
		user_infor("e:MNC:%s",Lm2minf.mnc);
	}
	memset(Lsiminf.ccid, 0, 21);
	u8t1 = AT_CCID(Lsiminf.ccid);
	if(u8t1)
	{
		if(u8t1 > 20)Lsiminf.ccidlen = 20;
		else Lsiminf.ccidlen = u8t1;
		if(u8t1 >= 6)
		{
			if('8' == Lsiminf.ccid[0] && '9' == Lsiminf.ccid[1] && '8' == Lsiminf.ccid[2] && '6' == Lsiminf.ccid[3] && '0' == Lsiminf.ccid[4] && '0' ==Lsiminf.ccid[5])
			{
			    Lsiminf.tsp = 1;
			}
			else if('8' == Lsiminf.ccid[0] && '9' == Lsiminf.ccid[1] && '8' == Lsiminf.ccid[2] && '6' == Lsiminf.ccid[3] && '0' == Lsiminf.ccid[4] && '1' ==Lsiminf.ccid[5])
			{
			    Lsiminf.tsp = 2;
			}
		}
		if(u8t1 >= 7)Lsiminf.business = Lsiminf.ccid[6] - '0';
		if(u8t1 >= 8)Lsiminf.simfun = Lsiminf.ccid[7] - '0';
		if(u8t1 >= 10)
		{
			Lsiminf.province = Lsiminf.ccid[8] - '0';
			Lsiminf.province = (Lsiminf.province << 4) + (Lsiminf.ccid[9] - '0');
		}
		if(u8t1 >= 12)
		{
			Lsiminf.year = Lsiminf.ccid[10] - '0';
			Lsiminf.year = (Lsiminf.year << 4) + (Lsiminf.ccid[11] - '0');
		}
		if(u8t1 >= 13)
		{
			Lsiminf.supplier = Lsiminf.ccid[12] - '0';
		}
		user_infor("e:CCID:%s",Lsiminf.ccid);
	}
	//Èç¹ûCCIDÓëFLASHÖĞ±£´æµÄ²»Í¬ÔòĞèÒªÖØĞÂ»ñÈ¡µç»°ºÅÂë ·ñÔòÖ±½ÓÊ¹ÓÃFLASHÖĞµÄ±£´æµÄºÅÂë
	
	if(1)
	{
		  memset(Lsiminf.tel, 0, 12);
	    	  u8t1 = AT_CNUM(Lsiminf.tel);
	    	  if(u8t1)
		 {
	    		if(u8t1 > 11)Lsiminf.tellen = 11;
	    		else Lsiminf.tellen = u8t1;
	    		user_infor("e:tel:%s",Lsiminf.tellen);
	    	  }
	    	  else{//SIM¿¨ÖĞÃ»ÓĞµç»°ºÅÂë ĞèÒªÍ¨¹ıÆäËû·½Ê½»ñµÃ
	   }
  }
 
  AT_SMSinit(); 
  user_infor("e:sim_information OK");
  Lm2mstatus.m2mstatus = 3;
  AT_delay(100);
}

u8 * m2m_imeiget(void){
	return (u8 *)Lm2minf.imei;
}
/*
*¶ÁÈ¡M2MĞÅÏ¢
*********************************************************/
void m2m_information(void){
	u8 u8t1;
	
	AT_CBC();
	user_infor("e:m2m_information >>");
	memset(Lm2minf.imei, 0, 16);
	u8t1 = AT_GSN(Lm2minf.imei);
	if(u8t1)
	{
		if(u8t1 > 15)Lm2minf.imeilen = 15;
		else Lm2minf.imeilen = u8t1;
		user_infor("e:IMEI:%s",Lm2minf.imei);
	}
	
	
  
	if(0 == AT_VER())
	{
		user_debug("i:AT_VER error");
		return;
	}
	
	if(AT_CIPMUX() != 0)
	{//2015/1/15 9:51  ²ÉÓÃ¶àÁ´½Ó·½Ê½
		user_debug("i:AT_CIPMUX error");
		return;
  	}
  
  //AT_SMSinit();
	Lm2mstatus.m2mstatus = 2;
	user_infor("e:m2m_information OK");
	AT_delay(100);
}


/**
*¼ì²âM2MµÄÍøÂç¸½×Å×´Ì¬
*Èç¹ûÒÑ¾­¸½×Åµ½GPRS£¬ĞèÒªÉèÖÃLm2mstatus.gprsbond²ÎÊı
*
***************************************************/
void m2m_gprsbond(void){
	u8 u8t1;
	user_infor("e:m2m_gprsbond >>");
	/*u8t1 = AT_CSQ();
	if(u8t1 < 4 ){//ĞÅºÅÌ«Èõ
		Lm2mstatus.gprsbond = 0;
		user_debug("m2m_gprsbond:CSQ low:%d", u8t1);
		AT_delay(500);
		return;
	}*/
	Lm2mstatus.csq = u8t1;
	AT_delay(300);
	if(0 == AT_CGREG()){//²éÑ¯ÍøÂç×¢²áĞÅÏ¢
		Lm2mstatus.gprsbond = 0;
		user_debug("i:m2m_gprsbond:CGREG error");
		AT_delay(500);
		return;
	}
	AT_delay(300);
	if(0 == AT_CGATT()){//²éÑ¯SIM¿¨ÊÇ·ñ¸½×ÅGPRS
		Lm2mstatus.gprsbond = 0;
		user_debug("i:m2m_gprsbond:CGATT error");
		return;
	}
	AT_delay(300);
	Lm2mstatus.gprsbond = 0x01;
	Lm2mstatus.m2mstatus = 4;
	user_infor("e:m2m_gprsbond OK");
}


/**
*m2mÁ¬½Óµ½GPRS
************************************************/
void m2m_gprslink(void){
	u8 u8t1;
	static u8 flag = 0;
	
	AT_CIPSHUT();//Ê×ÏÈ¹Ø±ÕÏÖ³¡  ²»¹ØĞÄÊÇ·ñ³É¹¦
	eat_sleep(1000);//µÈ´ı1Ãë
	user_debug("i:m2m_gprslink >>");
	if(1 == Lsiminf.tsp)
	{
		if(0 == AT_CSTT(0))
		{
			Lm2mstatus.gprslink = 0;
			user_debug("i:m2m_gprslink:CSTT error");
			AT_delay(500);
		  	return;
		}
	}
	else if(2 == Lsiminf.tsp)
	{
		if(0 == AT_CSTT(1))
		{
			Lm2mstatus.gprslink = 0;
			user_debug("i:m2m_gprslink:CSTT1 error");
			AT_delay(500);
		  	return;
		}
	}
	else
	{
		user_debug("i:m2m_gprslink:CSTT1 tsp[%d]", Lsiminf.tsp);
		if(0 == flag)
		{
			AT_CSTT(0);
			flag = 0x55;
		}
		else
		{
			AT_CSTT(1);
			flag = 0;
		}
	}
	AT_delay(400);
	if(0 == AT_CIICR())
	{
		Lm2mstatus.gprslink = 0;
		user_debug("i:m2m_gprslink:CIICR error");
		return;
	}
	AT_delay(400);
	u8t1 = AT_CIFSR(Lm2mstatus.localip);
	user_debug("i:AT_CIFSR:%s", Lm2mstatus.localip);
	if(u8t1)
	{
		if(u8t1 < 25)Lm2mstatus.localiplen = 25;
		Lm2mstatus.localiplen = u8t1;
	}
	else
	{
		Lm2mstatus.gprslink = 0;
		user_debug("i:m2m_gprslink:CIFSR error");
		return;
	}
	AT_delay(400);
	//Á¬½Ó·şÎñÆ÷
	if(0 == AT_CIPSTART("39.108.246.45",9999))//Lm2mstatus.seraddr, Lm2mstatus.port))
	{
		Lm2mstatus.gprslink = 0;
		user_debug("i:m2m_gprslink:CIPSTART error:%s[%d]","39.108.246.45",9999);//Lm2mstatus.seraddr,Lm2mstatus.port);
		return;
	}
	Lm2mstatus.gprslink = 1;
	Lm2mstatus.m2mstatus = 5;
	AT_delay(200);
	user_infor("e:m2m_gprslink OK");
}




u8 m2m_status_modemstart(void){
	return Lm2mstatus.modemstart;
}
u8 m2m_status_gprsbond(void){
	return Lm2mstatus.gprsbond;
}
u8 m2m_status_gprslink(void){
	return Lm2mstatus.gprslink;
}
u8 m2m_status(void){
	return Lm2mstatus.m2mstatus;
}

void m2m_statusSet(u8 status){
	Lm2mstatus.m2mstatus = status;
}
