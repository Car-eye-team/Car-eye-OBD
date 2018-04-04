/*
*
*开辟了一个大的内存区域，该区域允许整个APP使用，
*但同时只能有一个应用在使用
*使用完后需要释放
*
*使用该方式主要是由于eat_malloc可能存在申请失败的情况
*
*当前主要用于以下几个地方
*1、OBD数据缓冲区
*2、升级数据缓冲
*同一时间只能存储一类数据
*OBD数据缓冲
*     OBD数据缓冲分两级缓冲 BIG_MEM_MAX为1级缓存，当以及缓冲数据满或系统进入睡眠前需要将1级缓存数据保存到2级缓存
*     FS为2级缓存，大小为1.5M
****************************************/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "eat_modem.h"
#include "eat_interface.h"
#include "eat_uart.h"
#include "definedata.h"
#include "UartTask.h"
#include "AppUser1.h"
#include "AppUser2.h"
#include "OBDapi.h"
#include "db.h"
#include "ATapi.h"
#include "SVRapi.h"
#include "BigMem.h"


static u8 Lbigmemory[BIG_MEM_MAX];
static u8 LbigmemFlag = 0;/*
                            1 = 当前为OBD数据缓存
                            2 = 当前为升级数据缓存
                          */
u8 Lbigmemory_send[EAT_RTX_BUF_LEN_MAX];//大内存数据发送缓冲区
static _BIGMEM Lbigmem;

void bigmem_init(void)
{
	LbigmemFlag = 0;
	Lbigmem.dataindex = 0;
	Lbigmem.datalen = 0;
	Lbigmem.msgin = 0;
	Lbigmem.msgout = 0;
	Lbigmem.msgnum = 0;
}
/*
*内存使用申请，需要输入type为适用类型
*type = 2:升级数据
*       1:OBD数据（含GPS）
*如果是同一类数据可以重复申请该内存，但数据会从0地址开始存放
********************************************/
u8 *bigmem_get(u8 type){
	if(1 == type){
		if(2 == LbigmemFlag)return NULL;
	}
	if(2 == type){
		if(1 == LbigmemFlag)return NULL;
	}
	else return NULL;
	LbigmemFlag = type;
	return Lbigmemory;
}

void bigmem_free(void){
	LbigmemFlag = 0;
}

/*无条件将1级缓冲中到数据保存到2级缓冲中
*/
u8 bigmem_save(void){
	  if(Lbigmem.dataindex < 5)return 1;
	  user_debug("lilei-sleep and save bigbufdata\r\n");
	  db_filesave(Lbigmemory, Lbigmem.dataindex, 0);
		Lbigmem.dataindex = 0;
		Lbigmem.datalen = 0;
	  Lbigmem.msgin = 0;
	  Lbigmem.msgout = 0;
	  Lbigmem.msgnum = 0;
	  return 0;
}
/*OBD+GPS数据保存到1级缓冲区中
*1级缓冲区满有两个条件：
*   1、Lbigmemory中的数据满
*   2、_BIGMEM数据帧满
*****************************************/
u8 bigmem_obdgps_in(u8 *datain, u16 datalen)
{
	u16 datainindex;
  
  //user_infor("bigmem_obdgps_in:[%d-%d]", datalen, LbigmemFlag);
	if(NULL == datain || 0 == datalen || datalen >= 512)return 1;//每帧数据长度不超过512个字节
	if(2 == LbigmemFlag)return 2;//失败
	if(0 == LbigmemFlag)
	{
		user_debug("lilei--first fill LBigBUf\r\n");                                 //---lilei---add by lilei-2016-0613
		LbigmemFlag = 1;											//big_mem被用于保存OBD+gps数据
		Lbigmem.dataindex = 0;
		Lbigmem.datalen = 0;
	  	Lbigmem.msgin = 0;
	  	Lbigmem.msgout = 0;
	  	Lbigmem.msgnum = 0;
	}
	if(Lbigmem.datalen + datalen >= (DATA_LOG_FILE_MAX - 512))
	{//缓冲区满 需要将数据保存到2级缓冲区中
		//数据保存到2级缓冲区
		user_debug("lilei-BigMem data cnt >1024*100 save file\r\n");         //add by lilei--2016-0613
		db_filesave(Lbigmemory, Lbigmem.dataindex, 0);
		Lbigmem.dataindex = 0;
		Lbigmem.datalen = 0;
	  	Lbigmem.msgin = 0;
	  	Lbigmem.msgout = 0;
	  	Lbigmem.msgnum = 0;
	}
	Lbigmem.msg[Lbigmem.msgin].index = Lbigmem.dataindex;
	Lbigmem.msg[Lbigmem.msgin].len = datalen;
	memcpy((s8 *)&Lbigmemory[Lbigmem.dataindex], datain, datalen);
	Lbigmem.dataindex += datalen;
	if(Lbigmem.dataindex >= DATA_LOG_FILE_MAX)Lbigmem.dataindex = 0;
	Lbigmem.datalen += datalen;
	Lbigmem.msgin ++;
	if(Lbigmem.msgin >= BIG_MEM_MSG_MAX)Lbigmem.msgin = 0;
	Lbigmem.msgnum ++;
	if(Lbigmem.msgnum >= BIG_MEM_MSG_MAX)
	{//缓冲区数据满
		//数据保存到2级缓冲区
		user_debug("lilei-BigMem stareamer>1280 save file\r\n");					//add by lilei--2016-0613
		db_filesave(Lbigmemory, Lbigmem.dataindex, 0);
		Lbigmem.dataindex = 0;
		Lbigmem.datalen = 0;
	  	Lbigmem.msgin = 0;
	  	Lbigmem.msgout = 0;
	  	Lbigmem.msgnum = 0;
	}
	user_debug("i:bigmem_obdgps_in:[%d,%d,%d,%d,%d]",Lbigmem.msgnum,Lbigmem.dataindex,Lbigmem.datalen,Lbigmem.msgin,Lbigmem.msgout);
	return 0;
}

/*从二级缓存中读取数据
*
***************************************************/
u8 bugmem_obdgps_readfromfile(void)
{
	u32 filesize,fileindex;
	
	if(2 == LbigmemFlag)return 0;//失败
	if(0 == LbigmemFlag)
	{
		LbigmemFlag = 1;//big_mem被用于保存OBD+gps数据
		Lbigmem.dataindex = 0;
		Lbigmem.datalen = 0;
	  	Lbigmem.msgin = 0;
	  	Lbigmem.msgout = 0;
	  	Lbigmem.msgnum = 0;
	}
	filesize = db_fileread(Lbigmemory);
	if(0 == filesize)bigmem_free();
	else
	{
		if(0xa5 == Lbigmemory[0] && 0xa5 == Lbigmemory[1])
		{
			  //对数据进行格式化
			 Lbigmem.msgin = 0;
			 Lbigmem.msgout = 0;
			 Lbigmem.msg[Lbigmem.msgin].index = 0;
			 Lbigmem.msgnum = 0;
			 for(fileindex = 2; fileindex < filesize; fileindex ++)
			 {
			   	if(0xa5 == Lbigmemory[fileindex] && 0xa5 == Lbigmemory[fileindex +1])
				{
			   		Lbigmem.msg[Lbigmem.msgin].len =  fileindex - Lbigmem.msg[Lbigmem.msgin].index;
			   		Lbigmem.msgin ++;
			   		Lbigmem.msgnum ++;
			   		if(Lbigmem.msgnum >= BIG_MEM_MSG_MAX)return 0;
			   		Lbigmem.msg[Lbigmem.msgin].index = fileindex;
			   		if(Lbigmem.msgin >= BIG_MEM_MSG_MAX)break;
			   	 }
			   }
			 
			   Lbigmem.datalen = filesize;
			   Lbigmem.dataindex = fileindex;
			   user_debug("lilei --read db file msgnum=%d,datalen=%u,dataindex=%u\r\n",Lbigmem.msgnum, Lbigmem.datalen,Lbigmem.dataindex );
			   return Lbigmem.msgnum;
		}
		else
		{
			bigmem_free();
		}
	}
	return 0;
}

/*从1级缓冲区中读取数据，通常是APPuser1将数据从1级缓冲区中读取用于发送
*
********************************************/
u16 bigmem_obdgps_out(u8 *dataout)
{
	u16 datalen,msgoutlog, msgnumlog,datalenlog;
	
	if(NULL == dataout)return 0;
	if(1 == LbigmemFlag)
	{
		if(0 == Lbigmem.msgnum)
		{
			bigmem_free();
			return 0;
		}
		
		msgoutlog = Lbigmem.msgout;
		msgnumlog = Lbigmem.msgnum;
		datalenlog = Lbigmem.datalen;
		datalen = 0;
		while(1)
		{//组包 但一包数据不超过512个字节
			if(Lbigmem.msg[Lbigmem.msgout].len >= EAT_RTX_BUF_LEN_MAX)
			{
				user_debug("lilei--send LbigBuf len>512\r\n");	
		  	}
		  	else
			{
		  		if(datalen + Lbigmem.msg[Lbigmem.msgout].len < EAT_RTX_BUF_LEN_MAX)
				{
					user_debug("lilei--send LbigBuf-datalen=%d\r\n",Lbigmem.msg[Lbigmem.msgout].len);								//add by lilei-2016-0613
		  			memcpy((s8 *)&Lbigmemory_send[datalen], (s8 *)&Lbigmemory[Lbigmem.msg[Lbigmem.msgout].index], Lbigmem.msg[Lbigmem.msgout].len);
		  			datalen += Lbigmem.msg[Lbigmem.msgout].len;
		  		}
		  		else break;
		  	}
		  	if(Lbigmem.datalen >= Lbigmem.msg[Lbigmem.msgout].len)Lbigmem.datalen -= Lbigmem.msg[Lbigmem.msgout].len;
		  	else
			{//数据异常
		  		Lbigmem.dataindex = 0;
		    		Lbigmem.datalen = 0;
	      			Lbigmem.msgin = 0;
	      			Lbigmem.msgout = 0;
	      			Lbigmem.msgnum = 0;
		  		return 0;
		  	}
		  	Lbigmem.msgout ++;
			if(Lbigmem.msgout >= BIG_MEM_MSG_MAX)Lbigmem.msgout = 0;
			Lbigmem.msgnum --;
			if(0 == Lbigmem.msgnum)
			{
		      		bigmem_free();
		      		break;
		  	}
		}
		user_debug("i:bigmem_obdgps_out:[%d,%d,%d,%d]",Lbigmem.msgnum,Lbigmem.datalen,Lbigmem.msgout,datalen);
		if(0 == datalen)return 0;
		if(0 == AT_CIPSEND(Lbigmemory_send, datalen))
		{
			return datalen;	
		}
		else
		{//数据发送失败 
			Lbigmem.msgout = msgoutlog;
			Lbigmem.msgnum = msgnumlog;
			Lbigmem.datalen = datalenlog;
			return 0;//发送失败
		}	
	}
	else
	{
	   	if(db_filecheck())
		{
	   		if(0 == bugmem_obdgps_readfromfile())
			{
	   			user_debug("i:bigmem_obdgps_out1:[%d,%d,%d]",Lbigmem.msgnum,Lbigmem.msgin,Lbigmem.datalen);
	   			return 0;
	    		}
	    		user_debug("i:bigmem_obdgps_out2:[%d,%d,%d]",Lbigmem.msgnum,Lbigmem.msgin,Lbigmem.datalen);
	   		return 1;
	  	}
	   	return 0;
  	}
}

/*数据直接发送到服务器
*
*
*******************************************/
u8 bigmem_obdgps_tosvr(void)
{
	u16 datalen;
	
	if(1 == LbigmemFlag)
	{
		if(0 == Lbigmem.msgnum || Lbigmem.datalen < Lbigmem.msg[Lbigmem.msgout].len)
		{
			bigmem_free();
			return 0;
		}
		user_infor("e:bigmem_obdgps_tosvr:[%d-%d]", Lbigmem.msgnum, Lbigmem.msgout);
		AT_CIPSEND(&Lbigmemory[Lbigmem.msg[Lbigmem.msgout].index], Lbigmem.msg[Lbigmem.msgout].len);
		Lbigmem.msgout ++;
		if(Lbigmem.msgout >= BIG_MEM_MSG_MAX)Lbigmem.msgout = 0;
		Lbigmem.msgnum --;
		Lbigmem.datalen -= Lbigmem.msg[Lbigmem.msgout].len;
		if(0 == Lbigmem.msgnum)
		{
			bigmem_free();
		}
		return datalen;
	}
	else return 0;
}


