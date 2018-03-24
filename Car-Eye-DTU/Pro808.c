#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <rt_misc.h>

#include "eat_modem.h"
#include "eat_interface.h"
#include "eat_uart.h"
#include "definedata.h"
#include "UartTask.h"
#include "ATapi.h"
#include "AppUser1.h"
#include "AppUser2.h"
#include "SVRapi.h"
#include "Pro808.h"

u16 serialnumber=0;
_SendDataLoop SendDataLop;
u8 ProFrameHeadFg = 0;	
u16 ProFrameLen;
u8 ProTBuf[512];
u8 ProTempBuf[512];
sProPara  ProPara;
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

u8 GetXorSum(u8 *psrc, s16 len)
{
    s16 i = 0;
    u8 checksum = 0;
    
    for (i=0;i<len;i++)
    {
        checksum = checksum ^ psrc[i];
    }

    return checksum;
}

u32 ConvertLatOrLat(u32 value)
{
	u32 degree=0;
	float FDegree=0.0;


	
	degree=value/1000000UL;
	FDegree=degree + ((value / 1000000.0 - degree) * 100.0) / 60.0;
	degree=FDegree*1000000UL;
	return degree;
	


}


u16 Pro_Escape(u8* data,u16 datalen,u8* buf)
{
	u16 i,j;
	for(i = 0,j = 0; i < datalen;i ++)
	{
		if(data[i] == PROTOCOL_SIGN)
		{
			buf[j++] = PROTOCOL_ESCAPE;
			buf[j++] = PROTOCOL_ESCAPE_SIGN;
			
		}
		else if(data[i] == PROTOCOL_ESCAPE)
		{
			buf[j++] = PROTOCOL_ESCAPE;
			buf[j++] = PROTOCOL_ESCAPE_ESCAPE;
		}
		else
		{
			buf[j++] = data[i];
		}
	}
	memcpy(data,buf,j);
	return j;
}

u16 ProFrame_Pack(u8 *dec,u16 Cmd,sProPara* Para,u8* Tempbuf)
{
	u16 declen;
	sMessagehead* Msgheadptr;
	u8* Msgbody;
	sPositionbasicinfo* PosPtr;
	u32 TempLatOrLon=0;
	u16 ProFIndex=0;

	
	s8 TBuf[12]={'0','1','3','1','9','7','1','8','2','4','3','6'};
//	u8 tlen;



	//if(Sim808Deal.Sim808Step!=SIM8080_LOCATION_SUC)
	//{
	//	if((Cmd!=UP_Register)&&(Cmd!=UP_Authentication))
	//	{

	//		return 0x7f;

	//	}

	//}
	Msgheadptr = (sMessagehead*)&dec[1];

	Msgheadptr->id = BigLittleSwap16(Cmd);
	Msgheadptr->attribute.val = 0x0000;
	Msgheadptr->attribute.bit.encrypt = 0;	//不加密//
	Msgheadptr->attribute.bit.package = 0;
	//memset(tempbuf,0x00,12);                  add by lilei
	//memcpy(tempbuf,(u8*)(SerialNumberAddress),12);  add by lilei

	StringToBcd((s8*)Msgheadptr->phone,TBuf,6,0);
	serialnumber ++;
	Msgheadptr->serialnum = BigLittleSwap16(serialnumber);

	Msgbody = &dec[MSGBODY_NOPACKAGE_POS];
	declen = 12;

	switch(Cmd)
	{
	case UP_UNIRESPONSE://			0x0001		//终端通用应答//
		*(u16*)Msgbody = BigLittleSwap16(Para->u16Para3);	//应答流水号//
		Msgbody += 2;
		*(u16*)Msgbody = BigLittleSwap16(Para->u16Para4);	//应答ID//
		Msgbody += 2;
		*Msgbody = Para->u8Para1;	//结果//
		Msgbody ++;
		//MyPrintf("结果:%d\r\n",Para->u8Para1);
		Msgheadptr->attribute.bit.msglen = 5;
		Msgheadptr->attribute.val = BigLittleSwap16(Msgheadptr->attribute.val);
		declen += 5;
		break;
		
	case UP_HEARTBEAT://				0x0002		//终端心跳//
		Msgheadptr->attribute.bit.msglen = 0;	//心跳无消息体数据//
		break;
	case UP_REGISTER://				0x0100		//终端注册//
	       memset(Tempbuf,0x00,sizeof(Tempbuf));
		memcpy(Msgbody,Tempbuf,38);
		Msgbody+=38;
		declen+=38;
		Msgheadptr->attribute.bit.msglen = 38;
		Msgheadptr->attribute.val = BigLittleSwap16(Msgheadptr->attribute.val);	
		break;
	case UP_LOGOUT://				0x0101		//终端注销//
		break;
	case UP_AUTHENTICATION://		0x0102		//终端鉴权//
		//memcpy(Msgbody,Para->buf,Para->u8Para1);
		
		//Msgbody += Para->u8Para1;
		//declen += Para->u8Para1;
		//memcpy(Msgbody,(u8 * )&Sim808Deal.AutionBuf[1],Sim808Deal.AutionLen);
	
		//Msgbody +=Sim808Deal.AutionLen;
		//declen +=Sim808Deal.AutionLen;
		//Msgheadptr->attribute.bit.msglen = Sim808Deal.AutionLen;
		
		Msgheadptr->attribute.val = BigLittleSwap16(Msgheadptr->attribute.val);
		break;
	case UP_POSITIONREPORT://			0x0200		//位置信息汇报//
		




		PosPtr=(sPositionbasicinfo*)Msgbody;
		PosPtr->alarm.val = 0;
		PosPtr->state.val = 0;

		PosPtr->state.bits.acc = 1;
		PosPtr->state.bits.gpsen = 0;	//有GPS数据//
		PosPtr->state.bits.Alarm_en = 0;
	
		//PosPtr->state.bits.snlatitude =Lgpscur.latflag-1;
		//PosPtr->state.bits.ewlongitude= Lgpscur.longflag-1;
		//if(Lgpscur.enable==0x55)
		//{
		//	PosPtr->state.bits.location = 1;
		//}
		//else
		//{
		//	PosPtr->state.bits.location = 0;
		//}
		PosPtr->state.bits.trip_stat = 0;
		PosPtr->state.val = BigLittleSwap32(PosPtr->state.val);


	       //user_debug("Lgpscur.latitude=%d,Lgpscur.longitude=%d\r\n",Lgpscur.latitude,Lgpscur.longitude);
		


		   
		 
		//TempLatOrLon=ConvertLatOrLat(Lgpscur.latitude);
		PosPtr->latitude = BigLittleSwap32(TempLatOrLon);          //*10的6次方


		//TempLatOrLon=ConvertLatOrLat(Lgpscur.longitude);
		PosPtr->longitude = BigLittleSwap32(TempLatOrLon);	 //*10的6次方
		

		PosPtr->atitude = 0;
		//PosPtr->speed = BigLittleSwap16((u16)(Lgpscur.speed/10));	///100*10=1/10KM/h
		//PosPtr->direction = BigLittleSwap16((u16)(Lgpscur.angle/100));      //角度
		//PosPtr->time[0] = Lgpscur.Time808[0];
		//PosPtr->time[1] =  Lgpscur.Time808[1];
		//PosPtr->time[2] =  Lgpscur.Time808[2];
		//PosPtr->time[3] =  Lgpscur.Time808[3];
		//PosPtr->time[4] =  Lgpscur.Time808[4];
		//PosPtr->time[5] =  Lgpscur.Time808[5];
		declen+=sizeof(sPositionbasicinfo);
		Msgbody+=sizeof(sPositionbasicinfo);
		Msgheadptr->attribute.bit.msglen = sizeof(sPositionbasicinfo);
		Msgheadptr->attribute.val = BigLittleSwap16(Msgheadptr->attribute.val);
	
		break;
	case 0x0900:
		*Msgbody =0x43;
		Msgbody ++;
		declen ++;
		memcpy(Msgbody,Tempbuf,Para->u16Para3);
		Msgbody += Para->u16Para3;
		declen += Para->u16Para3;
		Msgheadptr->attribute.bit.msglen = Para->u16Para3;
		Msgheadptr->attribute.val = BigLittleSwap16(Msgheadptr->attribute.val);
		break;
	/*case UP_Para://					0x0104		//查询终端参数应答//
		break;
	case UP_ReadObdPids:
		Msgbody[0] = Cmd_ReadPids.cnt;
		Msgbody ++;
		declen ++;
		if(Cmd_ReadPids.cnt > 0)
		{
			memcpy(Msgbody,Para->buf,Para->u8Para1);
			Msgbody += Para->u8Para1;
			declen += Para->u8Para1;
		}
		
		Msgheadptr->attribute.bit.msglen = Para->u8Para1 + 1;
		
		Msgheadptr->attribute.val = BigLittleSwap16(Msgheadptr->attribute.val);
		break;
	case UP_Positionreport://			0x0200		//位置信息汇报//
		memcpy(Msgbody,Para->buf,Para->u16Para3);
		Msgbody += Para->u16Para3;
		declen += Para->u16Para3;
		Msgheadptr->attribute.bit.msglen = Para->u16Para3;
		Msgheadptr->attribute.val = BigLittleSwap16(Msgheadptr->attribute.val);
		break;
	case UP_Positionresponse://		0x0201		//位置信息查询应答//
		*(u16*)Msgbody = BigLittleSwap16(Para->u16Para3);	//应答流水号//
		Msgbody += 2;
		declen += 2;
		
		memcpy(Msgbody,Para->buf,Para->u16Para4);
		Msgbody += Para->u16Para4;
		declen += Para->u16Para4;
		
		Msgheadptr->attribute.bit.msglen = Para->u16Para4 + 2;
		Msgheadptr->attribute.val = BigLittleSwap16(Msgheadptr->attribute.val);
		break;
	case UP_Obdresponse:
		*(u16*)Msgbody = Para->u16Para3;	//应答流水号//
		Msgbody += 2;
		declen += 2;
		*(u16*)Msgbody = BigLittleSwap16(Para->u16Para4);	//ID//
		Msgbody += 2;
		declen += 2;
		
		memcpy(Msgbody,Para->buf,Para->u32Para5);
		Msgbody += Para->u32Para5;
		declen += Para->u32Para5;
		
		Msgheadptr->attribute.bit.msglen = Para->u32Para5 + 2;
		Msgheadptr->attribute.val = BigLittleSwap16(Msgheadptr->attribute.val);
		break;
	case UP_PositionTrans://			0x0203		//位置转中文信息//
		memcpy(Msgbody,Para->buf,Para->u16Para3);
		Msgbody += Para->u16Para3;
		declen += Para->u16Para3;
		Msgheadptr->attribute.bit.msglen = Para->u16Para3;
		Msgheadptr->attribute.val = BigLittleSwap16(Msgheadptr->attribute.val);
		break;
	case UP_Jihuo://				0x0208		//激活//
		memcpy(Msgbody,Para->buf,Para->u16Para3);
		Msgbody += Para->u16Para3;
		declen += Para->u16Para3;
		*Msgbody= '\0';
		Msgbody++;
		declen ++;
		Msgheadptr->attribute.bit.msglen = Para->u16Para3 + 1;
		Msgheadptr->attribute.val = BigLittleSwap16(Msgheadptr->attribute.val);
		break;
	case UP_DSendSmsReq://			0x0209		//设备发送短信请求//
		sms =(sSmsBuf*) Para->buf;
		memcpy(Msgbody,sms->Phone,strlen((const char*)sms->Phone));
		Msgbody += strlen((const char*)sms->Phone);
		declen += strlen((const char*)sms->Phone);
		*Msgbody= '\0';
		Msgbody++;
		declen ++;
		*Msgbody++ = sms->iDCS;
		declen ++;
		*(u16*)Msgbody = BigLittleSwap16(sms->len);
		Msgbody += 2;
		declen +=2;
		memcpy(Msgbody,sms->buf,sms->len);
		Msgbody += sms->len;
		declen += sms->len;
		
		Msgheadptr->attribute.bit.msglen = declen - 12;
		Msgheadptr->attribute.val = BigLittleSwap16(Msgheadptr->attribute.val);
		break;
		
	case UP_Incidentreport://		0x0301		//事件报告//
	case UP_Questionanswer://		0x0302		//提问应答//
	case UP_Infodemand://			0x0303		//信息点播/取消//
//#define 		UP_Heartbeat				0x0803		//存储多媒体数据检索应答//
	case UP_DataPass://				0x0900		//数据上行透传//
		*Msgbody++ = Para->u8Para1;	//类型//
		declen ++;
		memcpy(Msgbody,Para->buf,Para->u8Para2);
		Msgbody += Para->u8Para2;
		declen += Para->u8Para2;
		Msgheadptr->attribute.bit.msglen = Para->u8Para2 + 1;
		Msgheadptr->attribute.val = BigLittleSwap16(Msgheadptr->attribute.val);
		break;
	case UP_CarControlresponse://	0x0500		//车辆控制应答//
		break;
//#define 		UP_Heartbeat				0x0700		//行驶记录仪数据上报//
//#define 		UP_Heartbeat				0x0701		//电子运单上报//
//#define 		UP_Heartbeat				0x0702		//驾驶员身份信息采集上报//
//#define 		UP_Heartbeat				0x0800		//多媒体事件信息上传//
//#define 		UP_Heartbeat				0x0801		//多媒体数据上传//
//#define 		UP_Heartbeat				0x0901		//数据压缩上报//
//#define 		UP_Heartbeat				0x0A00		//终端RSA公钥//
	case UP_REQ_RTC:	//0x0F01		//终端开机同步请求消息////
		Msgbody[0] = 0;
		Msgbody[1] = 0;
		Msgbody[2] = 0;
		Msgbody[3] = 0;
		Msgbody[4] = 0;
		Msgbody[5] = 0;
		Msgbody[6] = 0;
		Msgbody[7] = 0;
		
		Msgbody += 8;
		declen += 8;
		Msgheadptr->attribute.bit.msglen = 8;
		Msgheadptr->attribute.val = BigLittleSwap16(Msgheadptr->attribute.val);
		break;
	case UP_UpdataBag_Req:	//			0x0FF2		//升级文件包请求//
		Msgbody[0] = UpdataCtl.ver;
		
		*(u32*)&Msgbody[1] = BigLittleSwap32(UpdataCtl.Curbag);
		Msgbody += 5;
		declen += 5;
		Msgheadptr->attribute.bit.msglen = 5;
		Msgheadptr->attribute.val = BigLittleSwap16(Msgheadptr->attribute.val);
		break;
	case UP_Updata_End://				0x0FF1		//升级完成//
		*(u32*)&Msgbody[0] = BigLittleSwap32(Para->u32Para5);
		*(u32*)&Msgbody[4] = BigLittleSwap32(Para->u32Para6);
		Msgbody[8] = Para->u8Para1;
		declen += 9;
		Msgbody += 9;
		Msgheadptr->attribute.bit.msglen = 9;
		Msgheadptr->attribute.val = BigLittleSwap16(Msgheadptr->attribute.val);
		break;*/
	}

	*Msgbody = GetXorSum(&dec[1],declen);
	declen ++;

	declen = Pro_Escape(&dec[1],declen,Tempbuf);
	dec[0] = PROTOCOL_SIGN;
	declen ++;
	dec[declen ++] = PROTOCOL_SIGN;

	  if(Cmd==UP_AUTHENTICATION)
	  {
		//SendBuffer.cmd=Cmd;
		//SendBuffer.len=declen;
		//SendBuffer.flag=TRUE;
		//memcpy(SendBuffer.buffer,dec,declen);
		return declen;
	
	  }
	user_debug("socket-send:");
	for(ProFIndex=0;ProFIndex<declen;ProFIndex++)
	{
		user_debug("%02X ",dec[ProFIndex]);

	}
	user_debug("\r\n");
	SendDataLop.SendBuf[SendDataLop.head].len=declen;
	SendDataLop.SendBuf[SendDataLop.head].flag=1;
	//SendDataLop.SendBuf[SendDataLop.head].cmd=Cmd;
	memcpy( SendDataLop.SendBuf[SendDataLop.head].buffer,dec,declen);
	AT_CIPSEND(SendDataLop.SendBuf[SendDataLop.head].buffer, SendDataLop.SendBuf[SendDataLop.head].len);
	SendDataLop.head++;
	
	
	if(SendDataLop.head>=LOOP_BUFFER_SIZE)
	{
			SendDataLop.head=0;
	}
	 	 
	
	return declen;
}


void ProFrameRec(u8 data)
{
	u8 Escape_fg = 0;
	u16 cmd;

	if(!ProFrameHeadFg)
	{
		if(data == PROTOCOL_SIGN)
		{
			ProFrameHeadFg = 1;
			ProFrameLen = 0;
			ProTBuf[ProFrameLen++] = data;
		}
	}
	else
	{
		if(!Escape_fg)
		{
			if(data == PROTOCOL_ESCAPE)
			{
				Escape_fg = 1;
				return;
			}
		}
		else
		{
			Escape_fg = 0;
			if(data == PROTOCOL_ESCAPE_SIGN)
			{
				data = PROTOCOL_SIGN;
			}
			else if(data == PROTOCOL_ESCAPE_ESCAPE)
			{
				data = PROTOCOL_ESCAPE;
			}
			else
			{
				user_debug("ESCAPE ERROR\r\n");
				ProFrameHeadFg = 0;
			}
			ProTBuf[ProFrameLen++] = data;
			return;
		}
	
		ProTBuf[ProFrameLen++] = data;
		
		if(data == PROTOCOL_SIGN)
		{
				if(ProFrameLen == 2)
				{
					ProFrameHeadFg = 1;
					ProFrameLen = 0;
					ProTBuf[ProFrameLen++] = data;
					return;
				}

				ProFrameHeadFg = 0;
				cmd = 0x0000;
				ProFramePrase(ProTBuf,ProFrameLen,&cmd);
				if(cmd != 0x0000)
				{
					/*slen = ProFramePack(TempProbuf,cmd,&ProPara,TempBuff);
					if(GsmGprsDataSend(TempProbuf,slen,0x00))
					{
						debugInf("回复\r\n");
					}*/
				}
		}
		else
		{
			if(ProFrameLen >= MAX_PROFRAMEBUF_LEN)
			{
				user_debug("ProFrameLen ERROR\r\n");
				ProFrameHeadFg = 0;
			}
		}
	}
}

void ProFramePrase(u8* FrameData,u16 Framelen,u16* ResId)
{
	sMessagehead* MsgHead;
	u8* Msgbody;
	u16 u16temp;
	u16 MsgLen;

	


	MsgHead = (sMessagehead*)&FrameData[1];
	MsgHead->attribute.val = BigLittleSwap16(MsgHead->attribute.val);
	
	if(MsgHead->attribute.bit.package)
	{
		Msgbody = &FrameData[MSGBODY_PACKAGE_POS];
	}
	else
	{
		Msgbody = &FrameData[MSGBODY_NOPACKAGE_POS];
	}

	MsgHead->serialnum = BigLittleSwap16(MsgHead->serialnum);
	MsgHead->id = BigLittleSwap16(MsgHead->id);
	MsgLen=(u16)(MsgHead->attribute.bit.msglen);
	switch(MsgHead->id)
	{
		case DOWN_UNIRESPONSE://			0x8001		//平台通用应答//
			 user_debug("Gernal 808 answer\r\n");
			//Sim808Deal.AnsWerFalg=TRUE;
			u16temp = *(u16*)&Msgbody[2];
			switch(BigLittleSwap16(u16temp))
			{
				case UP_AUTHENTICATION:
					if(Msgbody[4] == 0x00)
					{						
						user_debug("鉴权成功\r\n");								
					}
					else if(Msgbody[4] == 0x01)
					{
						user_debug("鉴权失败\r\n");
					}
					else if(Msgbody[4] == 0x02)
					{
						user_debug("鉴权消息有误\r\n");
					}
					else if(Msgbody[4] == 0x03)
					{
						user_debug("鉴权消息不支持\r\n");
					}
					break;
				case UP_REGISTER:
				       if(Msgbody[4] == 0x00)
				       {
						user_debug("Register success\r\n");
					}
					else
					{
						user_debug("register fail:%d\r\n",Msgbody[4]);

					}
					break;
				case UP_HEARTBEAT:
					
					break;		
		 }
		
		break;
	case DOWN_REGISTERRSPONSE://			0x8100		//终端注册应答//
		  
		  user_debug("Recive 808 Register\r\n");
		  //Sim808Deal.AutionLen=MsgLen-3;
		  //Sim808Deal.AutionBuf[0]=1;
		 // memcpy((u8 *)&Sim808Deal.AutionBuf[1],(u8 *)&Msgbody[3],Sim808Deal.AutionLen);
	        //Sim808Deal.AnsWerFalg=TRUE;
		break;
	
		
		
	default:
		break;
	}
	
}




u8   Up_Register()
{
	u8 UpTime=0;
	ProFrame_Pack(ProTBuf,UP_REGISTER,&ProPara,ProTempBuf);
	while(1)
	{
		SVR808_FameDeal();
		UpTime ++;
		if(UpTime > 2000)return 1;
		eat_sleep(5);
	}
	return 0;


}




u8  UP_Authentication()
{
	u8 UpTime=0;
	ProFrame_Pack(ProTBuf,UP_AUTHENTICATION,&ProPara,ProTempBuf);
	while(1)
	{
		SVR808_FameDeal();
		UpTime ++;
		if(UpTime > 2000)return 1;
		eat_sleep(5);
	}
	return 0;

}







u8 SVR808_FameDeal()
{

         u16 ProLen=0;
	  u16 SVRIndex=0;
	   u8 Flag7e=0;
	  u8 *DataPtr=NULL;
	  ProLen=MDM_DataToApp(&DataPtr);
	  Flag7e=0;
         if(ProLen<7||ProLen>512)
         {
		return 1;
	  }
	  for(SVRIndex=0;SVRIndex<ProLen;SVRIndex++)
	  {
	  	 user_debug("R:%02X ",DataPtr[SVRIndex]);
		 if((SVRIndex + 4 < ProLen)&& (0x7e== DataPtr[SVRIndex])&&(!Flag7e))
		 {

		        Flag7e=1;
			user_debug("recieve 808p\r\n,",DataPtr[SVRIndex]);
		
		 }
	  	if(Flag7e)
	       {
	  		 user_debug("%02X,",DataPtr[SVRIndex]);
	  		 ProFrameRec(DataPtr[SVRIndex]);
			 //Flag7e=0;
	  		
	  	}
	  }
}

