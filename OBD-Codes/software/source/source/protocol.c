
#include "includes.h"

//static u32 ResetState = 0xAA55AA55;

u8 TempBuf[1024];
static u8 TxhProBuf[512];
u8 SendDataBuffer[1024];
bool SendDataSign=FALSE;
static _ProDeal* ProTxh;

static sUartBuf  TxhUartBuf;

_sProCtl AppCtrl;

_ObdDeal ObdDeal;
_SendDataLoop SendDataLoop;
static u16 Accoibuf[10];
bool GsmPowerOffFlag=FALSE;
// 长整型大小端互换

_ProtocolDeal ProtocolDBuffer[ProBufSize]={
{0,"PRONONE-"},
{1,"PROPWM--"},
{2,"PROVPW--"},
{3,"ISO9141-"},
{4,"KWP5BPS-"},
{5,"KWP2000-"},
{6,"CANSTD5-"},
{7,"CANEXT5-"},
{8,"CANSTD2-"},
{9,"CANEXT2-"},
};
#define BigLittleSwap32(A)  ((((unsigned long)(A) & 0xff000000) >> 24) | \
							(((unsigned long)(A) & 0x00ff0000) >> 8) | \
							(((unsigned long)(A) & 0x0000ff00) << 8) | \
							(((unsigned long)(A) & 0x000000ff) << 24))




void ObdDealInital()
{

   memset(&ObdDeal,0x00,sizeof(_ObdDeal));
   memset(&SendDataLoop,0x00,sizeof(_SendDataLoop));
   memset(TempBuf,0x00,sizeof(TempBuf));
   ObdDeal.Obdsatus=OBDFIREOFF;
  

}

void Pro_Initial(void)
{
	
	memset(TempBuf,0x00,1024);
	
	memset(TxhProBuf,0x00,512);
	
	memset(TxhUartBuf.buf,0x00,UART_BUF_SIZE);

	TxhUartBuf.head = 0;
	TxhUartBuf.tail = 0;
	
	ProTxh = (_ProDeal*)TxhProBuf;	
	ProTxh->Com = COM_TXH_DEAL;
	ProTxh->step = 0;
}

void MileCheck(float curmile)
{
	float temp,tempoj,tempbj;
	
	if((curmile < saveset.oldSetMile) || (saveset.Mile < saveset.oldSetMile))
	{
		saveset.Mile_Percent = 1.025000;
	}
	else if(curmile == saveset.oldSetMile || saveset.Mile == saveset.oldSetMile)
	{
		
	}
	else
	{
		tempoj = saveset.Mile - saveset.oldSetMile;
		tempbj = curmile - saveset.oldSetMile;
		
		temp = tempbj/tempoj;
		saveset.Mile_Percent = saveset.Mile_Percent * temp;
	}
	
	//MyPrintf_tq("Mile_Percent:%f\r\n",saveset.Mile_Percent);
	if(saveset.Mile_Percent > 1.300000)
	{
		saveset.Mile_Percent = 1.025000;
	}
	else if(saveset.Mile_Percent < 0.700000)
	{
		saveset.Mile_Percent = 1.025000;
	}	
	saveset.oldSetMile = curmile;
}



void Pro_Txh_UartInt(void)
{
	u8 data;
	if(Uart_read(COM_TXH_DEAL,&data))
	{
		TxhUartBuf.buf[TxhUartBuf.head++] = data;

		if(TxhUartBuf.head >= UART_BUF_SIZE)
		{
			TxhUartBuf.head = 0;
		}

		if(TxhUartBuf.head == TxhUartBuf.tail)
		{
			TxhUartBuf.tail++;
			if(TxhUartBuf.tail >= UART_BUF_SIZE)
			{
				TxhUartBuf.tail = 0;
			}
		}
	}
}



bool Pro_Get_UartBuf_Data(u8* data,u8 com)
{
	sUartBuf* buf;

	if(com == COM_TXH_DEAL)
	{
		buf = &TxhUartBuf;
	}
	else
	{
		return FALSE;
	}

	if(buf->tail == buf->head)
	{
		return FALSE;
	}

	*data = buf->buf[buf->tail];
	buf->tail ++;
	if(buf->tail >= UART_BUF_SIZE)
	{
		buf->tail  = 0;
	}

	return TRUE;
}





u16 Pro_Pack(u8* buf,u8* src,u16 len,u8 com)
{
	u8* dec;

	dec = buf;
	
	dec[0] = 0x01;
	dec[1] = 0x01;		//头//

	memcpy(dec+2,src,len);

	dec[len+2] = GetXorSum(buf,len+2);

	//Uart_send(com, dec, len + 3);
	TraceHexStr(dec, len+3);
	return (len + 3);
}


u16 Pro_Pack_1(u8* buf,u8 Cmd,u8* src,u16 len,u8 com)
{
	u8* dec;

	dec = buf;
	
	dec[0] = 0x01;
	dec[1] = 0x01;		//头//

	dec[2] = Cmd;
	dec[3] = 0x00;

	dec[4] = len&0x00FF;
	dec[5] = (len >> 8)&0x00FF;
	

	memcpy(dec+6,src,len);

	dec[len+6] = GetXorSum(buf,len+6);

	TraceHexStr(dec, len+7);
	return (len + 7);
}

u16 Pro_Pack_Answer(u8* buf,u16 cmd1,u16 cmd2)
{
	
	  u16 length=0;
        u8 *dec;
	
		
        dec=buf;
        dec[length++]='#';
        memcpy((u8 *)&dec[length],&cmd1,2);
        length+=2;
	  length+=2;

	
	     switch(cmd2)
         {
	
	       case  0x0007:	//回复数据流//
	       	 *(u16*)&dec[length]=cmd2;
		    	 length+=2;
		    	 *(u16*)&dec[length]=AppCtrl.len+1;
		    	 length+=2;
		    	 dec[length++]=AppCtrl.n;
		    	 memcpy(&dec[length],AppCtrl.data,AppCtrl.len);
		    	 length+=AppCtrl.len;
	       	 break;
	       case  0x0008:	//回复VIN数据//
	       	 *(u16*)&dec[length]=cmd2;
		    	 length+=2;
		    	 dec[length]=AppCtrl.len;
		    	 length+=2;
		    	 memcpy(&dec[length],AppCtrl.data,AppCtrl.len);
		    	 length+=AppCtrl.len;
	       	 break;
		   case  0x0009:	//回复行程数据//																			  //上报当前行程状态//
				 Read_Trips_Data();
			     *(u16*)&dec[length]=cmd2;
			     length+=2;
			     *(u16*)&dec[length]=20;
			     length+=2;
			     *(float *)&dec[length]=(float)trips.tempmiles/1000;											//总的里程//
			     length+=4;
			     *(float *)&dec[length]=trips.tempoils;														//总的耗油量//
			     length+=4;
			     *(float *)&dec[length]=trips.boils;															//平均油耗//
			     length+=4;
			     *(float *)&dec[length]=temptrip.boil;															//当前行程的平均油耗//
			     length+=4;
			     *(float *)&dec[length]=ObdData.ssOil;															//瞬时油耗//												
			     length+=4;
				 break;			  
			case 0x000A:   //回复故障码数据//
			     *(u16*)&dec[length]=cmd2;
			     length+=2;
			     if(AppCtrl.n>0)
			     {

			    	 *(u16*)&dec[length]=AppCtrl.len;
			    	 length+=2;
			    	 dec[length++]=AppCtrl.n;
			    	 memcpy(&dec[length],AppCtrl.data,AppCtrl.len-1);
			    	 length+=(AppCtrl.len-1);
				 }
				 else
				 {
					 *(u16*)&dec[length]=AppCtrl.len;
					 length+=2;
					 dec[length++]=AppCtrl.n;		 		
				 }
			    
				 break;
			case 0x000B:   //回复清除故障码应答//
				 *(u16*)&dec[length]=cmd2;
			     length+=2;
			     *(u16*)&dec[length]=1;
			     length+=2;
			     dec[length++]=0;
				 break;
		    case 0x000C:
		         *(u16*)&dec[length]=cmd2;
			     length+=2;
			     *(u16*)&dec[length]=1;
			     length+=2;
			     //dec[length++]=Read443MResault;
		         break;
		    case 0x000D:
		    	 *(u16*)&dec[length]=cmd2;
			     length+=2;
			     *(u16*)&dec[length]=1;
			     length+=2;
			     dec[length++]=ObdDeal.Obdsatus;
		    	 break;
		    case 0x000E:
		         *(u16*)&dec[length]=cmd2;
		    	 length+=2;
		    	 *(u16*)&dec[length]=1;
		    	 length+=2;
		    	 dec[length++]=0;
		    	 break;
			default:
				  break;


       }
	 
 		    dec[length++]='$';
 		    length-=6;
 		    memcpy((u16 *)&dec[3],&length,2);
 		    length+=6;
 		    Uart_send(1,dec,length);
		    TraceHexStr(dec,length);
   
	    return 0;
}
		

u16 Pro_Pack_Obd(u8* buf,u16 cmd1,u16 cmd2)
{

	u16 length=0;
        u8 *dec;
	u8 ProTempBuffer[8];	
	u8 ProIndex=0;
        dec=buf;
        dec[length++]='#';
        memcpy((u8 *)&dec[length],&cmd1,2);
        length+=2;
        length+=2;
	
	 
	     switch(cmd2)
            {

			case 0x0001:   	//握手//
			      *(u16*)&dec[length]=cmd2;
			     length+=2;
			     *(u16*)&dec[length]=30;
			     length+=2;
				 
			     sprintf((char*)ProTempBuffer,"%8.5f",PowerV);
			     memcpy(&dec[length],ProTempBuffer,sizeof(ProTempBuffer));
			     length+=8;
			     dec[length++]=',';
			     for(ProIndex=0;ProIndex<ProBufSize;ProIndex++)
			     {

				     if(ProtocolDBuffer[ProIndex].ProtocolId==BspCtl->Protocol_type)
				      {
						 memcpy(&dec[length],ProtocolDBuffer[ProIndex].ProtocolBuffer,sizeof(ProtocolDBuffer[ProIndex].ProtocolBuffer));
						 length+=sizeof(ProtocolDBuffer[ProIndex].ProtocolBuffer);
						 break;
				       }
			     }
			     dec[length++]=',';			  
			     memset(ProTempBuffer,0x00,sizeof(ProTempBuffer));
			     sprintf((char*)ProTempBuffer,"%8.2f",ObdData.PidCycle.rpm);
			     memcpy(&dec[length],ProTempBuffer,sizeof(ProTempBuffer));
			     length+=8;
			     dec[length++]=','; 
			     memset(ProTempBuffer,0x00,sizeof(ProTempBuffer));
			     sprintf((char*)ProTempBuffer,"%3d",ObdData.PidCycle.vss); 
			     memcpy(&dec[length],ProTempBuffer,3);
			     length+=3;
		            break;
			case 0x0002: 	//上报点火状态//
			     *(u16*)&dec[length]=cmd2;
			     length+=2;
			     *(u16*)&dec[length]=1;
			     length+=2;
			     dec[length++]=0;
			     break;
			case 0x0003:	//上报熄火状态//
			     *(u16*)&dec[length]=cmd2;
			     length+=2;
			     *(u16*)&dec[length]=24;
			     length+=2;
			     *(u32 *)&dec[length]=trip.time;
			     length+=4;
			     *(u32 *)&dec[length]=trip.mile;
			     length+=4;
			     *(float *)&dec[length]=trip.oil;
			     length+=4;
			     *(float *)&dec[length]=trip.boil;
			     length+=4;
			     *(u32 *)&dec[length]=trip.idletime;
			     length+=4;
			     dec[length++]=trip.hspeed;
			     dec[length++]=trip.pspeed;
			     dec[length++]=trip.jiascnt;
			     dec[length++]=trip.jianscnt;
			     break;
		   	case  0x0004:	//主动上报故障码//																				
        		    	*(u16*)&dec[length]=cmd2;
	                    length+=2;
	                    *(u16*)&dec[length]=AppDtcreadCtl.dtc.Num*5+1;
	                    length+=2;
	                    dec[length++]=AppDtcreadCtl.dtc.Num;
	                    memcpy(&dec[length],&AppDtcreadCtl.dtc.dtc[0][0],AppDtcreadCtl.dtc.Num*5);
	                    length+=(AppDtcreadCtl.dtc.Num*5);			
                 	       break;	
         	  	case  0x0005:    //主动上报急加速//
		      	    	*(u16*)&dec[length]=cmd2;
			    	length+=2;
			    	*(u16*)&dec[length]=1;
			    	length+=2;
			   	dec[length++]=0;  	
		           	break;
	       	case  0x0006:	//主动上报急减速//
	    	 	  	*(u16*)&dec[length]=cmd2;
		    	  	length+=2;
		    	  	*(u16*)&dec[length]=1;
			  	length+=2;
		     	  	dec[length++]=0;
	    	     	   	break;
	       	case  0x000F:     
		 	 	break;
		   
 		    }
 		       dec[length++]='$';
 		       length-=6;
 		       memcpy((u16 *)&dec[3],&length,2);
 		       length+=6;
			 if(SendDataLoop.SendObdbuf[SendDataLoop.head].flag)
			{
				if(SendDataLoop.SendObdbuf[SendDataLoop.head].buffer[5]==0x03)                                                 //判断是不是熄火数据而不去覆盖//
				{					
					 SendDataLoop.head++;
					 Trace("遇到点火数据、不填充\r\n");
					 if(SendDataLoop.head>=OBDBUFFERSIZE)
 		       			{		       			
 		       				SendDataLoop.head=0;						
					 	}				
				}
			}
 		       SendDataLoop.SendObdbuf[SendDataLoop.head].len=length;
 		       SendDataLoop.SendObdbuf[SendDataLoop.head].flag=TRUE;
 		       memcpy( SendDataLoop.SendObdbuf[SendDataLoop.head].buffer,dec,length);		     
 		       SendDataLoop.head++;
 		       if(SendDataLoop.head>=OBDBUFFERSIZE)
 		       {
 		       		SendDataLoop.head=0;
 		       }		  	 
     	     	       return 0;
 }
	 
													
void ObdSendDataDeal()
{
   static u8 SendStep=0;
   static u32 SendTime=0;
   static s8 cnt=0;
   static bool flag=FALSE;

   if(!ObdDeal.GsmPowerStatus)
   {
   		SendStep=0;
   		flag=FALSE;
       	ObdDeal.SendFlag=FALSE;
		return;

   }
   if(flag)
   {
	switch(SendStep)
	{
		case 0:
		        Uart_send(UART_TXH_GSM,SendDataLoop.SendObdbuf[SendDataLoop.tail].buffer,SendDataLoop.SendObdbuf[SendDataLoop.tail].len);
			TraceHexStr(SendDataLoop.SendObdbuf[SendDataLoop.tail].buffer, SendDataLoop.SendObdbuf[SendDataLoop.tail].len);
			ObdDeal.SendFlag=TRUE;
			cnt=10;
			SendStep=1;
		        GetSysTick((u32 *)&SendTime);
			break;			  
		case 1:
			 if(!ObdDeal.SendFlag)
			 {
				Trace("收到应答\r\n");
				SendDataLoop.SendObdbuf[SendDataLoop.tail].flag=FALSE;
				SendDataLoop.SendObdbuf[SendDataLoop.tail].len=0;
				SendDataLoop.tail++;
				if(SendDataLoop.tail>=OBDBUFFERSIZE)
				{
					SendDataLoop.tail=0;

				}
				SendStep=0;
				flag=FALSE;
				return;

			 }
			 if(CheckSysTick((u32 *)&SendTime,3000))
			 {
			 	  MyPrintf("cnt=%d",cnt);
				  if(cnt>0)
				  {  
				      	   Trace("send again\r\n");
				     	   Uart_send(1,SendDataLoop.SendObdbuf[SendDataLoop.tail].buffer,SendDataLoop.SendObdbuf[SendDataLoop.tail].len);
				         TraceHexStr(SendDataLoop.SendObdbuf[SendDataLoop.tail].buffer, SendDataLoop.SendObdbuf[SendDataLoop.tail].len);
				       	    cnt--;
				  }
				  else
				  {    
				      	    GetSysTick((u32 *)&SendTime);
					    SendStep=2;					

				   }
			  }
			 break;
		case 2:
		 	   if(CheckSysTick((u32 *)&SendTime,1000))
		 	   {
                   			
                  			 SendStep=0; 
				   	ObdDeal.SendFlag=FALSE;
				  	 flag=FALSE;
					
		 	   }
		           break;
	        default :
			    SendStep=0;
			    ObdDeal.SendFlag=FALSE;
			    flag=FALSE;
   			    break;
            }
	  return ;
    }
   if(!SendDataLoop.SendObdbuf[SendDataLoop.tail].flag)
   {
		return;
   }
   flag=TRUE;
 
 
   



}
void Pro_Parse (_ProDeal* ProDeal,u8* idec)
{

    if(ObdData.Link_status != LINKING_STATUS_OK)
	{
		if((ProDeal->cmd == 0x0002)
		||(ProDeal->cmd  == 0x0003)
		||(ProDeal->cmd  == 0x0004)
		||(ProDeal->cmd  == 0x0005)
		||(ProDeal->cmd  == 0x0006)
		)
		{
			Pro_Pack_Answer(TempBuf,OBD_CMD,CAR_STATUS);
			return;
		}
	}
	switch(ProDeal->cmd)
	{

		case 0x0001:	//回应OBD 握手//
			 ObdDeal.HangShakeStatus=TRUE;
			 ObdDeal.SendFlag=FALSE;
			
			 break;
		case 0x0002:	//主动读取数据流//
			 if(!AppCtrl.lock)
			 {
				 AppCtrl.lock=TRUE;
				 AppCtrl.Cmd=ProDeal->cmd;
				 AppCtrl.len=ProDeal->len2;
				
				 memcpy(AppCtrl.buf,(u8 *)&ProDeal->buf[9],ProDeal->len2);
				 //TraceHexStr(AppCtrl.buf,ProDeal->len2);
			 }
		     	break;
		case 0x0003:	//主动读取VIN码//
			 if(!AppCtrl.lock)
			 {
				 AppCtrl.lock=TRUE;
			     	AppCtrl.Cmd=ProDeal->cmd;
			 }
		    	 break;
		case 0x0004:   //主动获取行程//
			 Pro_Pack_Answer(TempBuf,OBD_CMD,CMD_TRAVEL_STATUS);
			 break;
		case 0x0005:  //主动获取故障码//
		     if(!AppCtrl.lock)
		     {
				 AppCtrl.lock=TRUE;
				 AppCtrl.Cmd=ProDeal->cmd;
			 }
			 break;
		case 0x0006:  //清除故障码//
			 if(!AppCtrl.lock)
			 {
				 AppCtrl.lock=TRUE;
				 AppCtrl.Cmd=ProDeal->cmd;
			 }
			 break;
		case 0x0007:
			
		     break; 
		case 0x0008: //查询车辆状态//  
			 Pro_Pack_Answer(TempBuf,OBD_CMD,CAR_STATUS);
			 break;
		case 0x0009: //obd数据主动应答//
			 ObdDeal.SendFlag=FALSE;
			 break;
	      case 0x000A:  
	    	 
	    	 	break;
	   	default :
			 AppCtrl.Cmd=NULL;
			 break;

	}


}



void Pro_RecData(_ProDeal* ProDeal)
{
	u8 c;
	
	while(1)
	{
		if(Pro_Get_UartBuf_Data(&c,ProDeal->Com) == FALSE)
		{
			break;
		}

		ProDeal->buf[ProDeal->step] = c;
		//MyPrintf("%02X\r\n",c);
		switch (ProDeal->step)
		{
		case 0:
			if (c == 0x23)
			{
				ProDeal->step++;
			}
			break;
		case 1:
			if (c == 0x64)
			{
				ProDeal->step++;
			}
			else
			{
				ProDeal->step = 0; 
			}
			break;
		case 2:
			if (c == 0x01)
			{
				ProDeal->step++;
			}
			else
			{
				ProDeal->step = 0; 
			}
			break;
		case 3:
			ProDeal->len=c;
			ProDeal->step++;	//预留字//
			break;
		case 4:
			ProDeal->len += (u16)c*0x100 +6;
			ProDeal->step++;
			break;
		case 5:
			ProDeal->cmd=c;
			ProDeal->step++;
			break;
		case 6:
			ProDeal->cmd+= (u16)c*0x100;
			ProDeal->step++;
			 break;
		case 7:
			ProDeal->len2=c;
			ProDeal->step++;					
			break;
		case 8:
			ProDeal->len2 += (u16)c*0x100;
			ProDeal->step++;
			break;
		default:
			ProDeal->step++;
			//MyPrintf("ProDeal->step=%d\r\n",ProDeal->step);
			if(ProDeal->step >= ProDeal->len)
			{
			
				ProDeal->len -= 6;
				ProDeal->step = 0;
				Pro_Parse (ProDeal,TempBuf);
				return;
			}
			break;
		}
	}
}

void App_Task()
{


  static u32 AppTime=0;
  static u8 AppStep=0;

  switch(AppStep)
  {
	case 0:
		GetSysTick((u32 *)&AppTime);
		AppStep=1;
		break;
	case 1:
		  if(CheckSysTick((u32 *)&AppTime,10000))
  		 {  
    			 if(ObdDeal.GsmPowerStatus)
     			{
			  	if(!ObdDeal.SendFlag)
			  	{			  	   
				   	Pro_Pack_Obd(TempBuf,OBD_CMD,HAND_SHANKE);			 	   						   			 
				}
			 }
			AppStep=0;
  		}
	        break;
	default:
		AppStep=0;
		break;

  }
  ObdSendDataDeal();

}


 void SendDataBufClear()
 {

   	u8 i=0;
	bool FireOffFlag=FALSE;
	
      	for(i=0;i<OBDBUFFERSIZE;i++)
      	{

		if(SendDataLoop.SendObdbuf[i].flag)
		{

			if(SendDataLoop.SendObdbuf[i].buffer[5]==0x03)
			{
				FireOffFlag=TRUE;
				break;
			}
		}
         }

	if(FireOffFlag)
	{
	             if(i!=0)
	             {

			SendDataLoop.head=0;
			SendDataLoop.tail=0;
			SendDataLoop.SendObdbuf[SendDataLoop.head].flag=TRUE;
			SendDataLoop.SendObdbuf[SendDataLoop.head].len= SendDataLoop.SendObdbuf[i].len;
			memcpy(SendDataLoop.SendObdbuf[SendDataLoop.head].buffer, SendDataLoop.SendObdbuf[i].buffer,SendDataLoop.SendObdbuf[i].len);
			SendDataLoop.head=1;
		      }
		      else
		      {
			 	 SendDataLoop.tail=0;
				 SendDataLoop.head=1;
		      }
	             for(i=1;i<OBDBUFFERSIZE;i++)
	             {
				SendDataLoop.SendObdbuf[i].flag=FALSE;
		     }

	}
        else
        {
		memset(&SendDataLoop,0x00,sizeof(_SendDataLoop));  
	}

 }



void App_AccOutoCheckACCon(u16 V)

{
	static u8 Cnt = 0;
	u8 i,x,y;
	
    if(ObdDeal.Obdsatus!=OBDFIREOFF)
    {
    	return;
    }
 
	Accoibuf[0] = Accoibuf[1];
	Accoibuf[1] = Accoibuf[2];
	Accoibuf[2] = Accoibuf[3];
	Accoibuf[3] = Accoibuf[4];
	Accoibuf[4] = Accoibuf[5];
	Accoibuf[5] = Accoibuf[6];
	Accoibuf[6] = Accoibuf[7];
	Accoibuf[7] = Accoibuf[8];
	Accoibuf[8] = Accoibuf[9];
	Accoibuf[9] = V;

	if(Cnt < 10)
	{
		Cnt ++;
		return;
	}
	y = 0;
    	MyPrintf("Cnt=%d\r\n",Cnt);
	for(i = 0;i < 4; i ++)
	{
		for(x = 1,y=0;x<(9-i);x++)
		{
			if(Accoibuf[i] < (Accoibuf[i+x] - 55))
			{			
				y ++;
				MyPrintf("y=%d\r\n",y);	
			}
		}

		if(y > 4)
		{

	            if(!accstate)
	            {
				Trace("检测到电压跳变-------------点火\r\n");
				accstate = TRUE;
				ObdDeal.Obdsatus=OBDFIREON;
	            }         
		     break;
		}
	 
	}

   Cnt=0;
}

void Pro_Task(void)
{
	Pro_RecData(ProTxh);
}





