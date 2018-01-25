
/*
*************************************修改日志**********************************

*******************************************************************************
*/
#include "Includes.h"

/*
*******************************************************************************
* 常量定义
*******************************************************************************
*/


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

sUart   com1rx;
sUart   com1tx;

sUart   com2rx;
sUart   com2tx;

static u32 jSysTickCnt = 4294667296UL;

const  USART_TypeDef * uarts[4]={NULL,USART1,USART2,USART3};

static u16 PowerAdcValue;
bool OverFlag=FALSE;
u32 Encryption(void);

u16 BSP_PowerAdcValue(void)
{
	return PowerAdcValue;
}


u8 CheckAddress(u32 StartAddress)
{
	u16 i = 0,n = 256;
	u32 ptr;

	ptr = StartAddress;

	while(1)
	{
		if(*(u32*)(ptr + ((i + n)/2)*4) == 0xFFFFFFFF)
		{
			n = (i + n)/2;
		}
		else
		{
			i =  (i + n)/2;
		}

		if(i == n)
		{
			if(*(u32*)(ptr + i*4) == 0xFFFFFFFF)
			{
				i = i -1;
			}
			break;
		}
		else if(i + 1 == n)
		{
			break;
		}
	}
	return i;
}



void ReadOilMilePara(float* data,u32 StartAddress,u8 PageNum)
{
	u8 num;
	u32 temp;
	if(*(u32*)StartAddress == 0xFFFFFFFF)
	{
		if(*(u32*)(StartAddress+ Page_Size) == 0xFFFFFFFF)
		{
			FLASH_Unlock();
			FLASH_ErasePage(StartAddress);
			FLASH_ErasePage(StartAddress+Page_Size);
			*data = 0.000000;
			memcpy((s8*)&temp,(s8*)data,4);
			FLASH_ProgramWord(StartAddress,temp);
			FLASH_Lock();
		}
		else
		{
			num = CheckAddress(StartAddress+ Page_Size);
			*data = *(float*)(StartAddress+ Page_Size + num*4);
		}
	}
	else
	{
		num = CheckAddress(StartAddress);
		*data = *(float*)(StartAddress+ num*4);
	}
}

void WriteOilMilePara(float* data,u32 StartAddress,u8 PageNum)
{
	u8 num;
	u32 temp;
	if(*(u32*)StartAddress == 0xFFFFFFFF)
	{
		num = CheckAddress(StartAddress+ Page_Size);
		if(num != 255)
		{
			num ++;
			FLASH_Unlock();
			memcpy((s8*)&temp,(s8*)data,4);
			FLASH_ProgramWord(StartAddress+Page_Size+num*4,temp);
			FLASH_Lock();
		}
		else
		{
			FLASH_Unlock();
			memcpy((s8*)&temp,(s8*)data,4);
			FLASH_ProgramWord(StartAddress,temp);
			FLASH_ErasePage(StartAddress+Page_Size);
			FLASH_Lock();
		}
	}
	else
	{
		num = CheckAddress(StartAddress);
		if(num != 255)
		{
			num ++;
			FLASH_Unlock();
			memcpy((s8*)&temp,(s8*)data,4);
			FLASH_ProgramWord(StartAddress+num*4,temp);
			FLASH_Lock();
		}
		else
		{
			FLASH_Unlock();
			memcpy((s8*)&temp,(s8*)data,4);
			FLASH_ProgramWord(StartAddress+Page_Size,temp);
			FLASH_ErasePage(StartAddress);
			FLASH_Lock();
		}
	}
}

void ReadSavePara(_SaveSet* set)
{
	u8 x,j;
//	u32 data;
	u32* ptr;
	u32 address;
	u8* tptr;
	const u8 sibuf[12] = {'2','1','0','1','1','2','0','6','9','0','0','1'};
	
	FLASH_Unlock();
	if(*(u32*)(SaveParaAddress) == Page_UseFlag)
	{
		*set = *(_SaveSet*)(SaveParaAddress);
	}
	else if(*(u32*)(SaveParaAddress + Page_Size) == Page_UseFlag)
	{
		*set = *(_SaveSet*)(SaveParaAddress + Page_Size);
	}
	else
	{
		Trace("FLash 参数初始化\r\n");
		set->flag = Page_UseFlag;
		set->Encryptval = Encryption();
		set->OilMileage = set->Mile;
		set->MileageOil = set->Oil;
		set->EngineL = 1.800000;
		set->serialnum[0] = sibuf[0];
		set->serialnum[1] = sibuf[1];
		set->serialnum[2] = sibuf[2];
		set->serialnum[3] = sibuf[3];
		set->serialnum[4] = sibuf[4];
		set->serialnum[5] = sibuf[5];
		set->serialnum[6] = sibuf[6];
		set->serialnum[7] = sibuf[7];
		set->serialnum[8] = sibuf[8];
		set->serialnum[9] = sibuf[9];
		set->serialnum[10] = sibuf[10];
		set->serialnum[11] = sibuf[11];

		set->oldSetMile = 2000000.00;	//设置初值为200万公里，以作区别//
		set->Mile_Percent = 1.025000;	//里程校准系数,默认//
		set->K_Delay=0;
		set->K2000_Delay=0;
		FLASH_ErasePage(SaveParaAddress);
		FLASH_ErasePage(SaveParaAddress + Page_Size);
		tptr = (u8*)&set->flag;
		ptr = (u32*)tptr;
		x = sizeof(_SaveSet) / 4;
		for(j = 0;j < x; j++)
		{
			address = SaveParaAddress + j*4;
			FLASH_ProgramWord(address,*ptr);
			tptr +=4;
			ptr = (u32*)tptr;
		}

	}

	FLASH_Lock();

}

void WriteSavePara(u32* set)
{
	u16 x,j;
//	u32 data;
	u32* ptr;
	u8* tptr;
	
	FLASH_Unlock();
	
	if(*(u32*)(SaveParaAddress) == Page_UseFlag)
	{
		x = sizeof(_SaveSet) / 4;
		tptr = (u8*)set;
		ptr = (u32*)tptr;
		for(j = 0;j < x; j++)
		{
			FLASH_ProgramWord(SaveParaAddress + Page_Size + j*4,*ptr);
			tptr+=4;
			ptr = (u32*)tptr;
		}

		FLASH_ErasePage(SaveParaAddress);

	}
	else if(*(u32*)(SaveParaAddress + Page_Size) == Page_UseFlag)
	{
		x = sizeof(_SaveSet) / 4;
		tptr = (u8*)set;
		ptr = (u32*)tptr;
		for(j = 0;j < x; j++)
		{
			FLASH_ProgramWord(SaveParaAddress + j*4,*ptr);
			tptr+=4;
			ptr = (u32*)tptr;
		}

		FLASH_ErasePage(SaveParaAddress + Page_Size);
	}
	
	FLASH_Lock();
}










/*
******************************************************************************
功能描述：中断配置
输入参数；无
输出参数；无
返回参数：无
备注:     无
******************************************************************************
*/
void BSP_InterruptConfig(void)
{ 
  NVIC_InitTypeDef NVIC_InitStructure;


  /* Configure the Priority Group to 2 bits */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

  /* Configure the SysTick handler priority */
  //NVIC_SystemHandlerPriorityConfig(SystemHandler_SysTick, 0, 1);


  /* Enable the USART1 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
 
  /* Enable the USART2 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
   /* Enable the USART3 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);


  /* Enable the TIM3 Interrupt */
  /*
  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  */
}

/*
******************************************************************************
功能描述：系统时钟配置(配置1MS的系统时钟)
输入参数；无
输出参数；无
返回参数：无
备注:     无
******************************************************************************
*/
void BSP_SysTick_Config(void)
{
  /* Configure HCLK clock as SysTick clock source */
  SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);

  SysTick_Config(SystemCoreClock/1000);
 
}

/*
******************************************************************************
功能描述：IO口配置
输入参数；*Uart:串口号  Baud:波特率
输出参数；无
返回参数：无
备注:     无
******************************************************************************
*/
void BSP_UartInit(u8 PortNum, u32 Baud)
{
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

  	USART_InitStructure.USART_BaudRate = Baud;
  	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  	USART_InitStructure.USART_StopBits = USART_StopBits_1;
  	USART_InitStructure.USART_Parity = USART_Parity_No ;
  	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  	//USART_InitStructure.USART_Clock = USART_Clock_Disable;
  	//USART_InitStructure.USART_CPOL = USART_CPOL_Low;
  	//USART_InitStructure.USART_CPHA = USART_CPHA_2Edge;
  	//USART_InitStructure.USART_LastBit = USART_LastBit_Disable;

	switch (PortNum)
	{
		case 1:
		    /* Configure USART1 Tx (PA9) as alternate function push-pull */
		    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
		    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
		    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		    GPIO_Init(GPIOA, &GPIO_InitStructure);

		    /* Configure USART1 Rx (PA10) as input floating */
		    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
		    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
		    GPIO_Init(GPIOA, &GPIO_InitStructure);
			USART_Init(USART1, &USART_InitStructure);
			/* Enable the USART Transmoit interrupt: this interrupt is generated when the 
			USART1 transmit data register is empty */  
			//USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
			/* Enable the USART Receive interrupt: this interrupt is generated when the 
			 USART1 receive data register is not empty */
			USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
			/* Enable USART */
			USART_Cmd(USART1, ENABLE);
			break;
			
		case 2:
  			/* Configure USART2 RTS (PD4) and USART2 Tx (PD5) as alternate function push-pull */
  			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
		  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
		  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		  	GPIO_Init(GPIOA, &GPIO_InitStructure);

		  	/* Configure USART2 CTS (PD3) and USART2 Rx (PD6) as input floating */
		  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
		  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		  	GPIO_Init(GPIOA, &GPIO_InitStructure);
			/* Configure the USART */
			USART_Init(USART2, &USART_InitStructure);
			/* Enable the USART Transmoit interrupt: this interrupt is generated when the 
			USART1 transmit data register is empty */  
			//USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
			/* Enable the USART Receive interrupt: this interrupt is generated when the 
			 USART1 receive data register is not empty */
			USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
			/* Enable USART */
			USART_Cmd(USART2, ENABLE);
			break;
			
		case 3:
			
			/* Configure USART3 Tx (PC.10)as alternate function push-pull */
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
			GPIO_Init(GPIOB, &GPIO_InitStructure);
			  
			/* Configure USART3 Rx PC.11 as input floating */
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
			GPIO_Init(GPIOB, &GPIO_InitStructure);

  
			USART_Init(USART3, &USART_InitStructure);
			/* Enable the USART Transmoit interrupt: this interrupt is generated when the 
			USART1 transmit data register is empty */  
			//USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
			/* Enable the USART Receive interrupt: this interrupt is generated when the 
			 USART1 receive data register is not empty */
			USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
			//USART_ClearITPendingBit(USART3, USART_IT_RXNE);
			/* Enable USART */
			USART_Cmd(USART3, ENABLE);
			
			break;
		default:
			break;
  	}

}

/*
******************************************************************************
功能描述：读取指定串口缓冲区的数据到用户缓冲区
输入参数；nPort: 串口编号 0, 1, 2, 3其他值为非法
          iLen:  最大读取个数,最大254
          
输出参数；pDst 用户缓冲区字符串。
返回参数：-1表示失败，其他表示实际接收的字符数
备注:     最大接收254个字符
          缓冲区为环形
******************************************************************************
*/
s16 BSP_SendBufRead(u8 nPort, u8 *pDst)
{
	sUart   *pLoopBuf;

	switch( nPort)
	{
		case 1: 	
			pLoopBuf = &com1tx; 	
		    break;
		    
		case 2: 	
			pLoopBuf = &com2tx; 	
		    break;

		default:	
		    return -1;
	}
	
	if (pLoopBuf->iHead == pLoopBuf->iTail)
	{
		//Trace("No Send Data!\r\n");
		return -1;
	}
	*pDst = pLoopBuf->buf[pLoopBuf->iTail];
    pLoopBuf->iTail++;

	return 0;
}


void Uart_Send_Byte(u8 uart,u8 data)
{
	while(USART_GetFlagStatus((USART_TypeDef*)uarts[uart], USART_FLAG_TXE) == RESET)
	{
    	BSP_RESET_DOG();
    }
	USART_SendData((USART_TypeDef*)uarts[uart], data);
}

void Uart_send(u8 uart,u8* src,u16 len)
{
	while(len --)
	{
	    USART_SendData((USART_TypeDef*)uarts[uart], *src++);
	    while(USART_GetFlagStatus((USART_TypeDef*)uarts[uart], USART_FLAG_TXE) == RESET)
	    {
	    	BSP_RESET_DOG();
	    }
	}
}


bool Uart_read(u8 NUM,u8* data)
{
	bool ret = FALSE;
	if(USART_GetITStatus((USART_TypeDef*)uarts[NUM], USART_IT_RXNE) != RESET)
	{
		/* Clear the USART1 Receive interrupt */
		USART_ClearITPendingBit((USART_TypeDef*)uarts[NUM], USART_IT_RXNE);

		/* Read one byte from the receive data register */
		*data = uarts[NUM]->DR & 0xFF;
		ret = TRUE;
	}
	return ret;
}



/*
******************************************************************************
功能描述：看门狗初始化
输入参数；无
输出参数；无
返回参数：无
备注:     必须间隔0.4s挑用一次
******************************************************************************
*/
void BSP_WDG_Init(void)
{
  /* IWDG timeout equal to 350ms (the timeout may varies due to LSI frequency
     dispersion) -------------------------------------------------------------*/
  /* Enable write access to IWDG_PR and IWDG_RLR registers */
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

  /* IWDG counter clock: 32KHz(LSI) / 32 = 1KHz */
  IWDG_SetPrescaler(IWDG_Prescaler_128);

  /* Set counter reload value to 349 */
  IWDG_SetReload(0xfff);

  /* Reload IWDG counter */
  IWDG_ReloadCounter();

  /* Enable IWDG (the LSI oscillator will be enabled by hardware) */

  IWDG_Enable();

}

/*
******************************************************************************
功能描述：看门狗复位
输入参数；无
输出参数；无
返回参数：无
备注:     必须间隔0.4s挑用一次
******************************************************************************
*/
void BSP_RESET_DOG(void)
{
	IWDG_ReloadCounter();
	
}

/*
******************************************************************************
功能描述：系统滴答中断
输入参数；无
输出参数；无
返回参数：无
备注:     1ms一次
******************************************************************************
*/
void BSP_SysTick(void)
{
	jSysTickCnt ++;
	if (SoftDog != 0)
	{
		SoftDog--;
	}
	else
	{
		Uart_send(UART_DEBUG_PRINT, "SoftDog!\r\n",strlen("SoftDog!\r\n"));
		BSP_SysReset();
	}

	if(jSysTickCnt==0)
	{
		OverFlag=TRUE;
		Trace(" 主时钟开始清零\r\n");

	}
}


void GetSysTick(u32* n)
{
	*n = jSysTickCnt;
}

bool CheckSysTick(u32* n,u32 time)
{
	bool ret;
	u32 tick;
        u32 TempValue,MinusValue;

	

	ret = FALSE;
	tick = jSysTickCnt;
	TempValue=*n;

	
	MinusValue=(tick >= TempValue) ?(tick -TempValue)  : (0xFFFFFFFF - TempValue + tick );
	
	if(MinusValue >=  time)
	{
		ret = TRUE;

		*n += time;
		if(OverFlag)
		{
			MyPrintf("系统加Checksys==%d\r\n",*n );
		}
		
	}
	return ret;
}

bool CheckSysTicki(u32* n,u32 time)
{
	bool ret;
	u32 tick;
	u32 TempValue,MinusValue;


	
	ret = FALSE;
	tick = jSysTickCnt;
	TempValue=*n;
	MinusValue=(tick >= TempValue) ?(tick -TempValue)  : (0xFFFFFFFF - TempValue + tick );
	
	if(MinusValue >= time)
	{
		ret = TRUE;

		*n = tick;

		if(OverFlag)
		{

		
			MyPrintf("系统赋值Checksysii==%d\r\n",*n );

		}
	}
	return ret;
}

bool CheckSysTick_ck(u32* n,u32 time)
{
	bool ret;
	u32 tick;

	ret = FALSE;
	tick = jSysTickCnt;
	
	if(tick >= *n + time)
	{
		ret = TRUE;
		if(tick < (*n + time*2))
		{
			*n += time;
		}
		else
		{
			*n = tick;
		}
	}
	return ret;
}







void BSP_CanConfiguration(void)
{
	Driver_CAN_Init();

	//设置为普通模式
	Driver_CAN_SetMode(CAN_MODE_NORMAL);

	//设置过滤规则为“ID_LIST白名单”模式
	Driver_CAN_SetFilterRule(RULE_ID_LIST,0,0,0);

	//我们发送的CANID: 0x07DF
	//Multi_Frame_Request: 0x07E0

	//Request 29Bit CanId: 18 DB 33 F1

	Driver_CAN_FilterIdListAdd(0,0,0x07e8);

	//Driver_CAN_FilterIdListAdd(1,0,0x18DAF118);
	Driver_CAN_FilterIdListAdd(1,0,0x18DAF110);
}




/*
 ******************************************************************************
 * 功能描述：系统复位
 * 输入参数；无
 * 输出参数；无
 * 返回参数：无
 ******************************************************************************
 */
__asm void DisableInterrupts()
{
	CPSID i
	BX r14
}


void BSP_SysReset(void)
{
	NVIC_SystemReset();
	//NVIC_GenerateSystemReset();
	//NVIC_GenerateCoreReset();
}


void BSP_GpioConfig(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
  /* Configure PA.08,PA.12 as output pull-up */

 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

 

  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);                        					  //GSM POWER//

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);								//CAN POWER//
  

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  //GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;							//AD //
  GPIO_Init(GPIOA, &GPIO_InitStructure);



  GPIO_SetBits(GPIOA, GPIO_Pin_8);


  /*Config Gpio to reduce disturb and to improve Gps crq*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_7|GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;			
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  
  GPIO_ResetBits(GPIOA, GPIO_Pin_1);
  GPIO_ResetBits(GPIOA, GPIO_Pin_5);                                     //初始化默认的是GSM关闭电源//
  GPIO_SetBits(GPIOA, GPIO_Pin_6);      				   //初始化默认的是关闭CAN电源//
  GPIO_ResetBits(GPIOA, GPIO_Pin_7);
  GPIO_ResetBits(GPIOA, GPIO_Pin_15);
  
  GPIO_ResetBits(GPIOB, GPIO_Pin_0);
  GPIO_ResetBits(GPIOB, GPIO_Pin_1);
  GPIO_ResetBits(GPIOB, GPIO_Pin_3);
  GPIO_ResetBits(GPIOB, GPIO_Pin_4);
  GPIO_ResetBits(GPIOB, GPIO_Pin_5);
  GPIO_ResetBits(GPIOB, GPIO_Pin_6);
  GPIO_ResetBits(GPIOB, GPIO_Pin_7);
  GPIO_ResetBits(GPIOB, GPIO_Pin_8);
  GPIO_ResetBits(GPIOB, GPIO_Pin_9);
  GPIO_ResetBits(GPIOB, GPIO_Pin_12);
  GPIO_ResetBits(GPIOB, GPIO_Pin_13);
  GPIO_ResetBits(GPIOB, GPIO_Pin_14);
  GPIO_ResetBits(GPIOB, GPIO_Pin_15);

  
  
  GPIO_ResetBits(GPIOC, GPIO_Pin_13);
  GPIO_ResetBits(GPIOC, GPIO_Pin_14);
  GPIO_ResetBits(GPIOC, GPIO_Pin_15);

  



}


/*
******************************************************************************
功能描述：ADC检测配置  电源检测，ADC1 PA1 
输入参数；无
输出参数；无
返回参数：无
备注:     无
******************************************************************************
*/
#define ADC1_DR_Address    ((u32)0x4001244C)

u16 BSP_ScanAdcChannel(void)
{
	ADC_InitTypeDef ADC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);

	/* DMA channel1 configuration ----------------------------------------------*/
	ADC_Cmd(ADC1, DISABLE);
	DMA_Cmd(DMA1_Channel1, DISABLE);
	
	DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&PowerAdcValue;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = 1;

	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_DeInit(DMA1_Channel1);
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);

	/* ADC1 configuration ------------------------------------------------------*/
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;

	ADC_Init(ADC1, &ADC_InitStructure);

	/* ADC1 regular channel15 PC5 configuration */ 
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_71Cycles5);

	/* Enable ADC1 DMA */
	ADC_DMACmd(ADC1, ENABLE);

	/* Enable ADC1 */
	ADC_Cmd(ADC1, ENABLE);
	//ADC_TempSensorVrefintCmd(ENABLE);

	/* Enable ADC1 reset calibaration register */
	ADC_ResetCalibration(ADC1);
	
	/* Check the end of ADC1 reset calibration register */
	while(ADC_GetResetCalibrationStatus(ADC1));

	/* Start ADC1 calibaration */
	ADC_StartCalibration(ADC1);
	/* Check the end of ADC1 calibration */
	while(ADC_GetCalibrationStatus(ADC1));

	/* Start ADC1 Software Conversion */ 
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	
	/* Enable DMA channel1 */
	DMA_Cmd(DMA1_Channel1, ENABLE);

	/*
	(VCC/(2^12)) * ADC_ConvertedValue = Voltage
	*/

	return 0;
}


/*
******************************************************************************
功能描述：系统初始化
输入参数；无
输出参数；无
返回参数：无
备注:     无
******************************************************************************
*/

void Adc_PowerOff(void)
{
	ADC_Cmd(ADC1, DISABLE);
	DMA_Cmd(DMA1_Channel1, DISABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, DISABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,DISABLE);
}


void BSP_SysInit(void)
{

	/* ADCCLK = PCLK2/6 */
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	
    /* Enable DMA clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA 			/* Enable GPIOA, GPIOB, GPIOC, GPIOD, GPIOE and AFIO clocks */
	  						| RCC_APB2Periph_GPIOB
	  						| RCC_APB2Periph_GPIOC 
	  						| RCC_APB2Periph_ADC1
	         					| RCC_APB2Periph_TIM1			/* TIM1 Periph clock enable */
	  						| RCC_APB2Periph_USART1			/* Enable USART1 clocks */
	         					| RCC_APB2Periph_AFIO, ENABLE);
	
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM2
							| RCC_APB1Periph_USART3			/* Enable USART3 clocks */
	  						| RCC_APB1Periph_USART2, ENABLE);

	/*------------------- Resources Initialization -----------------------------*/

	/* GPIO Configuration */

//	SPI_FLASH_Init();

//	SPI_MEMS_Gpio_Init();

//	BSP_SPI_Init();
	BSP_GpioConfig();

	BSP_UartInit(UART_DEBUG_PRINT,9600);	 //GPS  串口//
	BSP_UartInit(UART_TXH_GSM, 115200);	    //通信串口//

   
	/* Configure the systick */    
	BSP_SysTick_Config();


	BSP_ScanAdcChannel();
	//Iso_Port_init();

	/*Configure the RTC*/
	//BSP_RTCInit();
	
	BSP_WDG_Init();

	/* Interrupt Configuration */
	BSP_InterruptConfig();

}



u32 Encryption(void)
{
	u32 CpuID[3];
	u32 temp;
	
	CpuID[0]=*(u32*)(0x1ffff7e8);
	CpuID[1]=*(u32*)(0x1ffff7ec);
	CpuID[2]=*(u32*)(0x1ffff7f0);

	CpuID[0] ^= 0xAA55AA55;
	CpuID[1] ^= 0x55AA55AA;
	CpuID[2] ^= 0xA5A5A5A5;

	
	temp = (CpuID[0] > CpuID[1]) ? (CpuID[0] - CpuID[1]) : (CpuID[1] - CpuID[0]);
	temp += (CpuID[1] > CpuID[2]) ? (CpuID[1] - CpuID[2]) : (CpuID[2] - CpuID[1]);
	temp += (CpuID[2] > CpuID[0]) ? (CpuID[2] - CpuID[0]) : (CpuID[0] - CpuID[2]);

	temp ^= 0xFFFFFFFF;

	//MyPrintf("Encryption:%d\r\n",temp);

	return temp;

}


void CheckEncryphtion(void)
{
	if(saveset.Encryptval != Encryption())
	{
		while(1)
		{
			BSP_RESET_DOG();
			TimeDly(65000);
			TimeDly(65000);
			TimeDly(65000);
			break;
		}

		while(1)
		{
			BSP_RESET_DOG();
			TimeDly(65000);
		}
	}
}



void SysTick_Deal()
{

	static u8 	SysTickStep=0;
	static u32 	SysTickTime=0;

	switch(SysTickStep)
	{

		case 0:

			GetSysTick((u32 *)&SysTickTime );
			SysTickStep=1;
			break;
		case 1:
			if(CheckSysTick((u32 *)& SysTickTime, 2*60*1000))          
			{
				if(jSysTickCnt>= TIME_40_DAYS)      
				{
					if(!ObdDeal.GsmPowerStatus)
					{
						jSysTickCnt=0;
						Trace("40 Days over System Reset\r\n");
						BSP_SysReset();	

					}

				}
				
				SysTickStep=0;
				Trace("\r\n查询一次时钟\r\n");

			}
			break;
		default:
			SysTickStep=0;
			break;




	}





}














