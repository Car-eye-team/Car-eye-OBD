/********************************************************************
 * Include Files
 ********************************************************************/
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <rt_misc.h>

#include "eat_modem.h"
#include "eat_interface.h"
#include "eat_periphery.h"
#include "eat_uart.h"
#include "definedata.h"
#include "UartTask.h"
#include "AppUser1.h"
#include "AppUser2.h"
#include "OBDapi.h"
#include "ATapi.h"
#include "SVRapi.h"
#include "BigMem.h"
#include "db.h"
#include "Pro808.h"

extern u32 Ltaskstartflag;
/*开辟一个内存区域用于动态申请
***************************************/
extern u8 APP_UPDATE_FLAGEX;
static u8 MDM_Link_Service_Flag = 0;/*
                                      M2M与服务器连接标志，M2M连接到服务器后设置为0x55，否则为0x00
                                    */
/*当时钟任务AppUser2检测到设备与服务器断开后调用该接口通知AppUser1设备已经与服务器断开
*/
static u8 mdm_unlinkset_flag = 0;
static u8 Lobd_start_mode = 0;

void MDM_unlinkset(u8 state)
{
	if(m2m_status() < 8)return;
	if(0x55 == mdm_unlinkset_flag)return;
	mdm_unlinkset_flag = 0x55;
	MDM_Link_Service_Flag = state;
	mdm_unlinkset_flag = 0;
}

static unsigned char Lpowr_lost_flag = 0;
unsigned char Lpowr_lost_flag_get(void){
	return Lpowr_lost_flag;
}

/*返回当前时间 s
*最大为524秒 超过后从0开始计算
****************************************/
u32 user_time(void)
{
	u32 time;
	
	time = eat_get_current_time();//31.25us 32/1000000
	return time * 32/1000000;
}
/*
void user_meminit(void){
	eat_bool mem_ini_flag;
	mem_ini_flag = eat_mem_init(Lbigmemory, BIG_MEM_MAX);
    if(EAT_TRUE == mem_ini_flag)
    {
        LbigmemFlag = 0x55; 
    }else
    {
        LbigmemFlag = 0x00;
    }   
}

u8 *usr_mallocEx(void){
	return Lbigmemory;
}
void *user_malloc(u32 size){
	if(0 == LbigmemFlag)user_meminit();
	if(0 == LbigmemFlag)return NULL;
	if(size >= BIG_MEM_MAX)return NULL;
	return eat_mem_alloc(size);
}

void *user_mfree(void *addr){
	if(NULL == addr)return NULL;
	if(0 == LbigmemFlag)return NULL;
	return eat_mem_free(addr);
}
*/
static _bdata Lbdata1;//GPS广播数据
static _bdata Lbdata2;//OBD广播数据
extern u8 Lbigmemory_send[EAT_RTX_BUF_LEN_MAX];//大内存数据发送缓冲区
void Lbdata1_insert(u8 id, u32 datain)
{
	u8 u8index;
	
	if(0x88 == Lbdata1.unitesdone)return;//这里需要保存
	for(u8index = 0; u8index < Lbdata1.num; u8index ++)
	{
		if(id == Lbdata1.units[u8index].id)
		{
			 Lbdata1.units[u8index].data = datain;
			 return;
		}
	}
	if(u8index >= Lbdata1.num && Lbdata1.num < BROAD_UNIT_MAX)
	{
		Lbdata1.units[Lbdata1.num].id = id;
		Lbdata1.units[Lbdata1.num].data = datain;
		Lbdata1.num ++; 
	}
}
void Lbdata1_insert_done(void){
	Lbdata1.unitesdone = 0x88;
	
}

void Lbdata2_insert(u8 id, u32 datain){
	u8 u8index;
	
	if(0x88 == Lbdata2.unitesdone)return;//这里需要保存
	for(u8index = 0; u8index < Lbdata2.num; u8index ++){
		if(id == Lbdata2.units[u8index].id){
			 Lbdata2.units[u8index].data = datain;
			 return;
		}
	}
	if(u8index >= Lbdata2.num && Lbdata2.num < BROAD_UNIT_MAX){
		Lbdata2.units[Lbdata2.num].id = id;
		Lbdata2.units[Lbdata2.num].data = datain;
		Lbdata2.num ++; 
	}
}
void Lbdata2_insert_done(void){
	Lbdata2.unitesdone = 0x88;
}

/*数据周期发送 0=未发送任何数据
*              1=成功发送
*/
u8 Lbdata_toserver(void)
{
	u8 backtosvr[256], unitindex, u8result,dataflag;
	u32 u32index,datalen,datalent,timeflag;
	static unsigned char Lbdata2num = 0;//OBD数据每分钟返回1次
	
	if(0x88 == Lbdata1.unitesdone || 0x88 == Lbdata2.unitesdone);
	else return 0;
	datalen = 0;
	dataflag = 0;
	if(0x88 == Lbdata1.unitesdone)
	{
		  Lbdata1.num = 0;
	    	 Lbdata1.unitesdone = 0x00;
		 user_debug("Fill in Gps 808\r\n");
		 ProFrame_Pack(ProTBuf,UP_POSITIONREPORT,&ProPara,ProTempBuf);
		
	 }
	  
  
 
	user_debug("i:Lbdata_toserver:[%d-%02x]", datalen,dataflag);
	return 1;
}
u8 MDM_Rx_buf[EAT_UART_MDM_RX_BUF_LEN_MAX + 1] = {0};//
u8 MDM_Rx_buf_Temp[EAT_UART_RX_BUF_LEN_MAX + 1] = {0};
u8 MDM_Tx_buf[EAT_UART_TX_BUF_LEN_MAX + 1] = {0};//该缓冲区用于MDM应用处理  
u8 MDM_App_buf[EAT_UART_TX_BUF_LEN_MAX + 1] = {0};//该缓冲区用于MDM应用处理  
static u16 MDM_Tx_buf_Num = 0;
static u16 MDM_Rx_buf_In = 0;
static _MDM mdm = {0};
u8 LMDM_SEND_ENABLE = 0;
static u8 SVR_EVENT_30 = 0;//每30秒产生的消息
static u8 SVR_EVENT_01 = 0;//每1秒产生的消息
static u8 SVR_EVENT_MOBILE_INFOR_UPDATE = 0;//更新基站数据
static u8 MDM_Tx_buf_flag = 0;//MDM_Tx_buf 使用标志
static u8 mdm_msgread_flag = 0;//MODEM数据
static u8 m2m_obd_heard = 0; //10s心跳周期

static u8 m2m_obd_heardflag = 0;
u8 m2m_obd_heard_set(u8 data){
	if(0x55 == m2m_obd_heardflag)eat_sleep(1);
	m2m_obd_heardflag = 0x55;
	if(1 == data){
		if(m2m_obd_heard < 10)m2m_obd_heard ++;
	}
	else m2m_obd_heard = 0;
	m2m_obd_heardflag = 0x00;
	return 0;
}

static u8 svr_event_mobileinfor_updateflag = 0;
u8 svr_event_mobileinfor_update(u8 state){
	if(0x55 == svr_event_mobileinfor_updateflag)eat_sleep(1);
	svr_event_mobileinfor_updateflag = 0x55;
	SVR_EVENT_MOBILE_INFOR_UPDATE = state;
	svr_event_mobileinfor_updateflag = 0x00;
	return 0;
}

static u8 svr_event30_enable_flag = 0;
void svr_event30_enable(u8 state)
{
	if(0x55 == svr_event30_enable_flag)eat_sleep(1);
	svr_event30_enable_flag = 0x55;
	SVR_EVENT_30 = state;
	svr_event30_enable_flag = 0;
}

static u8 svr_event01_enable_flag = 0;
void svr_event01_enable(u8 state){
	if(0x55 == svr_event01_enable_flag)eat_sleep(1);
	svr_event01_enable_flag = 0x55;
	SVR_EVENT_01 = state;
	svr_event01_enable_flag = 0;
}
/*************************************************************************
*DES   : MDM消息结构体初始化
*        modem返回数据到MDM_Rx_buf中，该结构体用于保存每一帧的信息
*INPUT : NULL
*RETURN: NULL
*********************************************************************************/
void mdm_Init(void){
	u8 u8t1;
	
	MDM_Rx_buf_In = 0;
	
	mdm.MDMmsgNum = 0;
	mdm.MDMmsgIn = 0;
	mdm.MDMmsgOut = 0;
	
	MDM_Tx_buf_Num = 0;
	
	LMDM_SEND_ENABLE = 0;
	MDM_Tx_buf_flag = 0;
	mdm_msgread_flag = 0;
	SVR_EVENT_MOBILE_INFOR_UPDATE = 0;
	
	m2m_obd_heard = 0;
	
	Lbdata1.unitesdone = 0;
	Lbdata2.unitesdone = 0;

	Lobd_start_mode=0;       										

/*清接收缓冲区
*为了防止modem发送的数据被异常清除，该接口必须由APP调用
*低层接口不允许调用
*该接口不支持重入，因此不允许在中断接口中调用，也不适合用于按时间片切换的系统
****************************************/
void MDM_RxBufClr(void){
	MDM_Tx_buf_Num = 0;
	MDM_Rx_buf_In = 0;
	mdm.MDMmsgNum = 0;
	mdm.MDMmsgIn = 0;
	mdm.MDMmsgOut = 0;
}


/*用于优先处理系统级指令
*通讯中断等
*该接口只提供MDM_DataToApp调用
*RETURN: 非0 = 系统指令 已经处理
         0 = 非系统指令
***************************************/
u8 MDM_SystemCommand(u8 *data, u16 datalen){
	u16 index;
	
	if(NULL == data || 0 == datalen)return 0;
	for(index = 0; index < datalen; index ++)
	{
		if(0xa5 == *(data + index) && 0xa5 == *(data + index +1))
		{
			//user_debug("SystemC:%02x-%02x", *(data + index + 4),*(data + index +5));
			app_svrlink(0);
			break;
		}
	}
	index = 0;
	if('C' == *(data + index) && 'L' == *(data + index + 1) && 'O' == *(data + index + 2) && 'S' == *(data + index + 3) && 'E' == *(data + index + 4))
	{
		//网络已经断开 需要重新连接
		m2m_statusSet(4);
		user_debug("i:Restart:close");
		return 1;
	}
	return 0;
}



/*
*DES   : 数据从MDM_Rx_buf读到MDM_App_buf中
*INPUT : NULL
*RETURN: 数据个数
*/
u16 MDM_DataToApp(u8 **dataptr)
{
	u16 datalen,dataindextemp;
	u16 index,smsindex;
	
	if(0 == mdm.MDMmsgNum)return 0;
	dataindextemp = 0;
	memset(MDM_App_buf, 0, EAT_UART_TX_BUF_LEN_MAX);
	//dataindextemp = mdm.msg[mdm.MDMmsgOut].MDMmsg_Index;
	for(datalen = 0; datalen < mdm.msg[mdm.MDMmsgOut].MDMmsg_Datalen && datalen < EAT_UART_TX_BUF_LEN_MAX; datalen ++)
	{
		MDM_App_buf[datalen] = MDM_Rx_buf[mdm.msg[mdm.MDMmsgOut].MDMmsg_Index + dataindextemp];
		dataindextemp ++;
		if(mdm.msg[mdm.MDMmsgOut].MDMmsg_Index + dataindextemp >= EAT_UART_MDM_RX_BUF_LEN_MAX)
		{
			mdm.msg[mdm.MDMmsgOut].MDMmsg_Index = 0;
			dataindextemp = 0;
		}
	}
	mdm.MDMmsgOut ++;
	if(mdm.MDMmsgOut >= MDM_MSG_MAX)mdm.MDMmsgOut = 0;
	if(0x55 == mdm_msgread_flag)eat_sleep(1);
	mdm_msgread_flag = 0x55;
	mdm.MDMmsgNum --;
	mdm_msgread_flag = 0x00;
	index = 0;
	for(index = 0; index < datalen; index ++)
	{
		if(0x0d == MDM_App_buf[index] || 0x0a == MDM_App_buf[index]);
		else break;
	}
  	if(MDM_SystemCommand(&MDM_App_buf[index], datalen - index) != 0)
	{
  		return 0;
  	}
  	if(datalen > 4)
	{//不同的应答M2M会一次性返回 因此需要进行分帧处理  提取需要的内容
      		for(smsindex = 0; smsindex < datalen -4; smsindex ++)
		{
      			if('+' == MDM_App_buf[smsindex] && 'C' == MDM_App_buf[smsindex+1 ] && 'M' == MDM_App_buf[smsindex+ 2] && 'T' == MDM_App_buf[smsindex + 3])
			{
      				if((0 == smsindex) || (0x0d == MDM_App_buf[smsindex -1] || 0x0a == MDM_App_buf[smsindex -1]))
				{
      		    			index = smsindex;//提取短信信息
      		    			break;
      	  			}
      			}
      		}
  	}
  	if('+' == MDM_App_buf[index] && 'C' == MDM_App_buf[index+1 ] && 'M' == MDM_App_buf[index+ 2] && 'T' == MDM_App_buf[index + 3])
	{//有短信来
		//短信处理
		    user_debug("i:SMS0:%s",(s8 *)&MDM_App_buf[index]);
		    SVR_SMSdeal(&MDM_App_buf[index]);
		    return 0;
	}
  
	*dataptr = &MDM_App_buf[index];
	//user_debug("MDM-LEN:[%d-%d]", datalen, index);
	datalen = datalen - index;
  //debug_hex("MDM:", *dataptr, datalen);
	//user_debug("MDM[%d]:%s <<",datalen,MDM_App_buf);
	return datalen;
}



void Lobd_start_mode_set(unsigned char state){
	Lobd_start_mode = state;
}

/********************************************************************
 * APP与modem通讯
 * modem向app发送数据时app_main首先会得到消息，在发送"MR"消息到app_modem
 * app_modem将完成数据接收
 *                    处理
 *                    回应
 * app_main也会发送主动发送消息给app_modem要求app完成某些操作，格式:Sxx 
 *                                                                  |_ S为APP发出的指令 xx为指令代号
 * app_main也会根据OBD数据通过app_modem发送到服务器，格式为:Oxx
 ********************************************************************/
 extern void TastStartSet_01(u8 flag);
void app_modem(void *data)
{
  	u8 *dataptr;
	EatEvent_st event;
	u32 eventnum;
	u8 u8result,u8t1;
	u8 testflag = 0;
	u16 datalen;
	u8 errornum,gpsonlineenbale;
	u8 ADCWARM,ADCNUM;
	u32 testtime;
	
  

  	ADCWARM = 0x55;//默认报警禁止  2016/3/10 15:07
  	ADCNUM = 0;
  	user_debug("i:app_modem start");
  	Lobd_start_mode = 0;
  	AT_Init(db_svr_addr1get(), db_svr_port1get());
  	svr_event30_enable(0);
  	svr_event01_enable(0);
  	testflag = 0;
  	TastStartSet_01(1);
  	errornum = 0;
  	testtime = 0;
  	gpsonlineenbale = 0;
  	Lpowr_lost_flag = 0;
	while(EAT_TRUE)
	{
		if(m2m_obd_heard >= 5)
		{
			obd_heard();
			m2m_obd_heard_set(0);
		}
		eventnum = eat_get_event_num_for_user(EAT_USER_1);
		if(eventnum || 1 == system_status_get())
		{
		   eat_get_event_for_user(EAT_USER_1, &event);
		   if(event.event == EAT_EVENT_USER_MSG || 1 == system_status_get())
		   {
          		if(1 == system_status_get() || (0x01 == event.data.user_msg.len && 'S' == *(event.data.user_msg.data)))
			{//进入睡眠
      	      			u8result = 0;
      	      			errornum = 0;
      	   
      	      			for(datalen = 0; datalen < 1800; datalen ++)
				{//发送缓冲区数据  最多连续发送1800条 时间最多为1小时
	  	       	    if((0x00 == (u8result & 0x01)) && (0 == SVR_Bigmem_send()))
				    {
	  	       	     		bigmem_save();
	  	       	    		u8result |= 0x01;
	  	       	    }
	  	       	    if((0x00 == (u8result & 0x02)) /*&& (anydata_databackout() != 0)*/)
				    {//数据发送结束
	  	       	    		errornum ++;//连续5次数据发送失败则认为失败  保证缓冲区中的数据能全部发送到平台
	  	       	    		if(errornum >= 3)u8result |= 0x02;
	  	       	    }
	  	       	    else errornum = 0;
	  	       	    if(0x03 == (u8result & 0x03))break;
	  	            	    eat_sleep(2000);
	  	       	}
		  	       //anydata_EVT_ENGINE_OFF();                                                  //add by lilei-2016-0823
	      	     		AT_CIPSHUT();
	      	     		AT_CMGDA();//睡眠前删除所有短信
			       m2m_statusSet(3);//3-->0
			       gpsonlineenbale = 0;
			       
			       
			       system_subtask_status_set(0x01);
			       user_debug("i:Mode-sleep");
			       eat_sleep_enable(EAT_TRUE);
			       db_gps_cellsave();
			       db_gps_save();
			       
			       while(1)
				{
				        eat_get_event_for_user(EAT_USER_1, &event);
				        if(0x01 == event.data.user_msg.len && 'U' == *(event.data.user_msg.data))break;
			       }
			       gpsonlineenbale = 0;
			       eat_sleep_enable(EAT_FALSE);
			       user_debug("i:Mode-weak up");
			   }
			}
		}
		if(0 == m2m_status())
		{
			MDM_RxBufClr();
			m2m_startcheck();
		}
		if(1 == m2m_status()){
			m2m_information();
		}
		if(2 == m2m_status())
		{
			sim_information();
		}
		if(3 == m2m_status())
		{
			m2m_gprsbond();
		}
		if(4 == m2m_status())
		{
			  u8result = 0;
			  m2m_gprslink();
		}
		if(5 == m2m_status())
		{
			if(0 == Up_Register())
			{
				//user_debug("Register Success\r\n");
				m2m_statusSet(6);
				errornum = 0;
			}
			else
			{
			   	errornum ++;
			   	if(errornum >= 3)
			   	{
			   		m2m_statusSet(4);
					user_debug("-set status :4\r\n");
			   		errornum = 0;
			   	}
		  	}
		}
		if(6 == m2m_status())
		{
			//if(gpsonlineenbale != 0x55)
			//{//2015/6/3 16:06 fangcuisong
			 //   	u8result = gps_assist_online_from_zgex();
			 //   	if(0 == u8result)
			  // 	{
			//	    gpsonlineenbale = 0x55;
			//gps_assist_toOBD();
			   //	}
				  
			//}
			if(0 == UP_Authentication())
			{
				errornum = 0;
				user_debug("Authentication success\r\n");
				m2m_statusSet(7);
			}
			else
			{
			  	errornum ++;
			  	if(errornum >= 3)
				{
			   		m2m_statusSet(4);
			   		errornum = 0;
			   	}
			}
		}
		if(7 == m2m_status())
		{
			m2m_statusSet(8);
			MDM_unlinkset(0x55);		
		}		
		obd_datadeal();//OBD数据处理
		gps_tosvr();//GPS数据返回
		if(m2m_status() > 6)
		{
			  //add by lilei-2016-0823
			obd_update_keep();
			if(m2m_status() > 7)
			{
				if(0 == MDM_Link_Service_Flag)
				{//服务器与M2M已经断开 需要重连
					m2m_statusSet(3);
				}
			
			}
			//bigmem_obdgps_tosvr();//返回缓冲区的数据 时间先后顺序没关系 服务器可以处理
			if(0x55 == SVR_EVENT_30)
			{//每30秒需要处理的事情
				//SVR_heard();
				svr_event30_enable(0);
			}
		}
		if(mdm.MDMmsgNum != 0)
		{//数据处理
			for(u8t1 = 0; u8t1 < MDM_MSG_MAX; u8t1 ++)
			{
			    SVR808_FameDeal();
			    //datalen = MDM_DataToApp(&dataptr);
			    //if(datalen)
			    //{
				 //   u8result = SVR_FameDeal(dataptr, datalen);
				    //if(0x7f == u8result);//非OBD数据 继续处理
				    //else break;
			    //}
			    //else break;
		  	}
		}
		else eat_sleep(5);//最小睡眠时间为5MS
		
		if(0x55 == SVR_EVENT_01)
		{//每1秒需要处理的事情
			ADCNUM ++;					 
	  		if(ADCNUM >= 5 && 0 == APP_UPDATE_FLAGEX)
			{
				m2m_ccidread();
				user_debug("send heart 808\r\n");
				ProFrame_Pack(ProTBuf,UP_HEARTBEAT,&ProPara,ProTempBuf);
	  			if(m2m_status() > 7)
				{				
	  		    		if(0 == Lobd_start_mode)
					{
						
	  		    			back2OBD_2Bytes(0x8e,0x02); 
				    	}
				  
			  	}
	  			if(1 == AT_CADC())
				{//电压异常 需要报警
	  				if(0 == ADCWARM)
					{
						
	  			    		Lpowr_lost_flag = 0x55;
	  			    		ADCWARM = 1;
	  		  		}
					else
					{	

					}
	  			}
	  			else
				{
	  				ADCWARM = 0;
	  			}
	  			ADCNUM = 0;
	  		}
	  		if(1 == ADCWARM && m2m_status() > 6)
			{//发送电压异常报警信息
	  			db_obd_save();
	  			obd_cmd8d(1);
	  			ADCWARM = 0x55;//报警信息只发送1次 再次发送需要设备重新启动或从睡眠中唤醒
	  		
	  		}
	  	
			if(0 == APP_UPDATE_FLAGEX)
			{
			    if(0 == Lbdata_toserver())
		           {//数据发送到服务器
			    	//未发送数据
			    		if(m2m_status() > 7)
					{//发送缓冲区数据
			    	    		SVR_Bigmem_send();
			      		}
			    	} 
		  	}
			svr_event01_enable(0);
	   }
	  AT_SMSinit();//该接口中的功能必须执行成功 否则循环执行 直到成功
	  if(m2m_status() >= 8 && 0 == testflag)
	  {//测试
			//SVR_test(1);
			//SVR_test(2);
			testflag ++;
	  }
   }
	user_debug("i:app_modem end");
}




/**************************************************************************
*APP与MODEM通讯
**********************************************************************************/
/*
 * 功能描述：读取MODEM返回的数据 数据保存到MDM_Rx_buf中
 * 输入参数：NULL
 * 输出参数：NULL
 * 返回参数：0=无数据
 *           N=接收到的数据个数
 * 外部调用：主线程检测到EAT_EVENT_MDM_READY_RD时调用该接口
 * 注意事项：该接口被主线程调用，MDM_Rx_buf_Temp数据接收
*/
static unsigned char MDM_read_debug = 0;
void MDM_read_debugset(void){
	MDM_read_debug = 0x55;
}
u16 MDM_read(void)
{
	u16 len,u16index,u16t1;
	u8 recnum;
	
	if(mdm.MDMmsgNum >= MDM_MSG_MAX)
	{
		user_debug("i:MDM_read:buf-full");
		return 0;
	}
	recnum = 0;
	while(1)
	{
	    memset(MDM_Rx_buf_Temp, 0, EAT_UART_RX_BUF_LEN_MAX);
	    len = eat_modem_read(MDM_Rx_buf_Temp, EAT_UART_RX_BUF_LEN_MAX);
	    if(len != 0)
	    {
	    	//user_debug("M:%s", MDM_Rx_buf_Temp);
	    	//if(0x55 == MDM_read_debug)user_debug("MODEM[%d-%02x,%02x,%02x,%02x,%02x,%02x]",len,MDM_Rx_buf_Temp[0],MDM_Rx_buf_Temp[1],MDM_Rx_buf_Temp[2],MDM_Rx_buf_Temp[3],MDM_Rx_buf_Temp[4],MDM_Rx_buf_Temp[5]);
	    	for(u16t1 = 0; u16t1 < len; u16t1 ++)
		{
	    		if((u16t1 + 6 < len)&& 0x0d == MDM_Rx_buf_Temp[u16t1] && 0x0a == MDM_Rx_buf_Temp[u16t1+1] &&  0x01 == MDM_Rx_buf_Temp[u16t1+2] && 0x00 == MDM_Rx_buf_Temp[u16t1+3] && 0x00 == MDM_Rx_buf_Temp[u16t1+4] && 0x00 == MDM_Rx_buf_Temp[u16t1+5])
			{
	    		   	user_debug("i:===========================");
	    		   	//anydata_CONN_RES();                         //add by lilei-2016-0823
	    		} 
	    	}
	    	//debug_hex("MODEM:", MDM_Rx_buf_Temp, len);
	    	mdm.msg[mdm.MDMmsgIn].MDMmsg_Datalen = len;
	    	mdm.msg[mdm.MDMmsgIn].MDMmsg_Index = MDM_Rx_buf_In;
	    	for(u16index = 0; u16index < len; u16index ++)
		{
	    		MDM_Rx_buf[MDM_Rx_buf_In] = MDM_Rx_buf_Temp[u16index];
	    		MDM_Rx_buf_In ++;
	    		if(MDM_Rx_buf_In >= EAT_UART_MDM_RX_BUF_LEN_MAX)MDM_Rx_buf_In = 0;
	    	}
	    	mdm.MDMmsgIn ++;
	    	if(mdm.MDMmsgIn >= MDM_MSG_MAX)mdm.MDMmsgIn = 0;
	    	
	    	
	    	if(mdm.MDMmsgNum < MDM_MSG_MAX)
		{
	    		if(0x55 == mdm_msgread_flag)eat_sleep(1);
	    		mdm_msgread_flag = 0x55;
	    		mdm.MDMmsgNum ++;
	    		mdm_msgread_flag = 0x00;
	    	}
	    	
	    }
	    else break;
	    eat_sleep(5);
	    recnum ++;
	    if(recnum > 16)break;//如果连续接收到8K数据则直接返回
      }
	return len;
}


/*
 * 功能描述：发送数据到MODEM
 * 输入参数：data = 需要发送的数据
             datalen= 发送的数据个数 不超过512
 * 输出参数：NULL
 * 返回参数：0=发送失败
 *           N=发送的数据个数
 * 外部调用：需要时调用
 * 注意事项：该接口会自动在发送数据结尾+0x0d 0x0a 因此应用程序不需要增加该字符
*/
u16 MDM_write(u8 *data, u16 datalen){
	u16 overtime;
	
	if(NULL == data || datalen >= (EAT_UART_TX_BUF_LEN_MAX - 8))return 0;
	//user_infor("MDM_write>>%s", data);
	//if(0x55 == MDM_read_debug)user_debug("MDM_write[%d-%02x,%02x,%02x,%02x]<<",datalen,*(data+3),*(data+4),*(data+5),*(data+6));
	if(MDM_Tx_buf_Num != 0 || 0x55 == MDM_Tx_buf_flag){
		while(1){
			eat_sleep(5);
			if(0 == MDM_Tx_buf_Num && MDM_Tx_buf_flag != 0x55)break;
			overtime ++;
			if(overtime > 200)return 0;
		}
	}
	
	MDM_Tx_buf_flag = 0x55;
	MDM_Tx_buf_Num = datalen;
	memset(MDM_Tx_buf, 0, EAT_UART_TX_BUF_LEN_MAX);
	memcpy(MDM_Tx_buf, data, datalen);
	MDM_Tx_buf[MDM_Tx_buf_Num ++] = 0x0d;
	MDM_Tx_buf[MDM_Tx_buf_Num ++] = 0x0a;
	MDM_Tx_buf_flag = 0x00;
	
	return datalen;
}
/*不带0x0d,0x0a
*********************************************/
u16 MDM_write1(u8 *data, u16 datalen){
	u16 overtime;
	
	if(NULL == data || datalen > EAT_UART_TX_BUF_LEN_MAX)return 0;
	if(MDM_Tx_buf_Num != 0 || 0x55 == MDM_Tx_buf_flag){
		while(1){
			eat_sleep(5);
			if(0 == MDM_Tx_buf_Num && MDM_Tx_buf_flag != 0x55)break;
			overtime ++;
			if(overtime > 200)return 0;
		}
	}
	//debug_hex("MDM_write1>>", data, datalen);
	if(0x55 == MDM_read_debug)user_debug("i:MDM_write1[%d-%02x,%02x,%02x,%02x]<<",datalen,*(data+3),*(data+4),*(data+5),*(data+6));
	MDM_Tx_buf_flag = 0x55;
	memset(MDM_Tx_buf, 0, EAT_UART_TX_BUF_LEN_MAX);
	MDM_Tx_buf_Num = datalen;
	memcpy(MDM_Tx_buf, data, datalen);
	MDM_Tx_buf_flag = 0x00;
	return datalen;
}
/*
*/
u8 MDM_writeEx(void){
	u16 len;
	
	if(MDM_Tx_buf_Num >= EAT_UART_TX_BUF_LEN_MAX)
	{
		MDM_Tx_buf_Num = 0;
		return 0;
	}
	

	if(MDM_Tx_buf_Num && 0x00 == LMDM_SEND_ENABLE)
	{
		debug_hex("MDM_writeEx>>",MDM_Tx_buf,MDM_Tx_buf_Num);
		
			
		len = eat_modem_write(MDM_Tx_buf, MDM_Tx_buf_Num);
		if(len < MDM_Tx_buf_Num)
		{//发送缓冲满
			user_debug("i:MDM_Tx_Buff full");
			LMDM_SEND_ENABLE = 0x55;
		}
		else
		{
		 	if(0x55 == MDM_Tx_buf_flag)eat_sleep(1);
		 	MDM_Tx_buf_flag = 0x55;
			MDM_Tx_buf_Num = 0;
		 	MDM_Tx_buf_flag = 0x00;
		}
		
	}
	return MDM_Tx_buf_Num;
}

u8 MDM_writeEnable(void){
	user_infor("e:MDM_writeEnable>>");
	LMDM_SEND_ENABLE = 0;
	return 0;
}

