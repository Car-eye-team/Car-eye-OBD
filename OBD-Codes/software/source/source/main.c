
#include "includes.h"
_SaveSet  saveset;
float PowerV = 12.000000;
u32 SoftDog=60000;
bool accstate = FALSE;



void PowerCheck(bool entry)
{
	static u32 Cjiffies=0;
	u16 temp;

	static bool Getfg = FALSE;

	static u16 data[28];
	static u8 cnt;

	u8 i ;
	bool ischanged;//设计跳出条件

	static u8 EgOffDly = 0;
	static u8 EgOnDly = 0;

	

	
	if(!CheckSysTicki((u32 *)&Cjiffies,50))
	{
		return;
	}

	temp = BSP_PowerAdcValue();

	if(entry)
	{
		Getfg = FALSE;
	}

	if(!Getfg)
	{
		Getfg = TRUE;
		memset(data,0x00,28);
		cnt = 0;
	}

	if(cnt < 28)
	{
		data[cnt++] = temp;
		return;
	}

	Getfg = FALSE;

	while(1)
	{
		ischanged =FALSE;
		for(i =0 ; i < 27; i ++)
		{
			if(data[i] > data[i+1])
			{
				temp = data[i+1];
				data[i+1] = data[i];
				data[i] = temp;
				ischanged =TRUE;
			}
		}
		if(!ischanged)
		{
			break;
		}
	}
	
//	for(i = 0; i < 28; i ++)
//	{
//		MyPrintf("%d\r\n",data[i]);
//	}
	
	temp = 0;
	for(i = 0; i < 8; i ++)
	{
		temp += data[i + 10];
	}

	temp = temp >> 3;
	
	//MyPrintf("%d\r\n",temp );


	PowerV = (float)(temp) * 0.000820;
	PowerV = PowerV*11;
	
	MyPrintf("Power:%fV\r\n",PowerV );


  	if (PowerV  > 13.1)
	{
		EgOffDly = 0;
		if (!accstate)
		{
			EgOnDly++;
			if (EgOnDly >= 10)   
			{
				EgOnDly = 0;
				accstate = TRUE;
 				ObdDeal.Obdsatus=OBDFIREON;
				CAN_POWER_ON;
				Trace("电压检测出:Acc On!\r\n");
			}
		}
	}
	else
	{
		EgOnDly = 0;
		if (accstate)
		{
			EgOffDly++;
			if (EgOffDly >= 10)
			{
				EgOffDly = 0;
				Trace("电压检测出:Acc Off!\r\n");
				if(ObdData.Link_status != LINKING_STATUS_OK)
				{
				    Trace("电压检测熄火\r\n");
					accstate = FALSE;
					ObdDeal.Obdsatus=OBDFIREOFF;
					ObdDeal.AccOffGsmPowerStep=0;
					CAN_POWER_OFF;
					
				}	
			}
		}	
	}
}

int main(void)
{


	


	
	BSP_SysInit();
	BspObdii_initial();
	Obd_App_Initial();
	Pro_Initial();
	ObdDealInital();
	MyPrintf("\r\ncar-eye-OBDII-1.0.0\r\n");
	MyPrintf("版本: %s\r\n",VER);
	MyPrintf("发布日期: [%s][%s]\r\n\r\n",__TIME__,__DATE__);
	memset((s8*)&saveset,0x00,sizeof(_SaveSet));
	ReadSavePara(&saveset);
	ObdData.EngineL = 1.6;
	PowerCheck(TRUE);
	BSP_RESET_DOG();
	SoftDog=60000;


	
	while(1)
	{    	
	     
	    PowerCheck(FALSE);
	    Obd_task();
	    Pro_Task();
	    App_Task();
	   // GpsTask();
	    BSP_RESET_DOG();
	    SoftDog=60000;

	  	
	}
}










