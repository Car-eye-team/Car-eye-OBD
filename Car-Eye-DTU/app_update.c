/*
*APP软件升级
*
*接收来自服务器的数据
*根据当前数据类型完成升级，如：M2M的APP升级
*                              OBD的APP升级等
************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rt_misc.h>

#include "eat_modem.h"
#include "eat_interface.h"
#include "definedata.h"
#include "app_update.h"
#include "OBDapi.h"
#include "BigMem.h"
#include "db.h"

static _APPUPDATE Lappupdate;
static u32 APP_DATA_RUN_BASE; //app运行地址
static u32 APP_DATA_STORAGE_BASE;  //app升级程序保存地址

void update_init(void){
	Lappupdate.flag = 0;
	Lappupdate.datalen = 0;
	Lappupdate.dataindex = 0;
	Lappupdate.data = 0;
}


#define LICENSE_FILE L"C:\\License"
u8 update_do(void)
{
	u32 app_space_value;
	u8 u8result;
	s32 fpr,seekRet;
	u32 filesize,readLen;
	eat_fs_error_enum fserror;
	
	if(Lappupdate.flag < 1)return 1;
	
	if(0 == Lappupdate.datalen || NULL == Lappupdate.data)
	{
		Lappupdate.flag = 0;
		user_debug("i:update_do len error:%d", Lappupdate.datalen);
		return 2;
	}
	
	if(UPDATE_M2M_LICENSE == Lappupdate.flag)
	{//License升级
		fpr =  eat_fs_Open(LICENSE_FILE, FS_READ_WRITE | FS_CREATE);
	  	if(fpr<EAT_FS_NO_ERROR)
		{
		    user_debug("i:LICENSE_FILE fileopen error");
		    update_end();
		    return 4;
	  	}
	  	eat_fs_Seek(fpr,0,EAT_FS_FILE_BEGIN);
	  	fserror = eat_fs_Write(fpr, Lappupdate.data, Lappupdate.datalen, &filesize);
	  	if(EAT_FS_NO_ERROR!= fserror|| Lappupdate.datalen != filesize)
		{
		   	user_debug("i:LICENSE_FILE data write error");
		   	update_end();
		   	return 5;
	  	}
	  	else
		{
	     		fserror = eat_fs_Commit(fpr);
	     		if(EAT_FS_NO_ERROR != fserror ){
	  	    	user_debug("i:LICENSE_FILE commit error");
	  	    	update_end();
		      return 6;
	     	}
	     	eat_fs_Close(fpr);
	     	user_debug("i:LICENSE_FILE update OK");
	     	db_update_save(Lappupdate.flag, Lappupdate.name, Lappupdate.datalen, Lappupdate.cs);
	     	update_end();
		   return 0;
	  }
	}
	
	u8result = db_update_save(Lappupdate.flag, Lappupdate.name, Lappupdate.datalen, Lappupdate.cs);
	
	if(u8result != 0){
	  user_debug("i:M2M update db_update_save error[%d-%s-%d]",u8result,Lappupdate.name,Lappupdate.datalen);	
		return u8result;
	}
	user_debug("i:M2M update OK[%d-%s-%d]",u8result,Lappupdate.name,Lappupdate.datalen);
	
	Lappupdate.flag = 0;
	APP_DATA_RUN_BASE = eat_get_app_base_addr();//获取app地址
	app_space_value = eat_get_app_space();//获取app空间大小
	APP_DATA_STORAGE_BASE = APP_DATA_RUN_BASE + (app_space_value >> 1);
	//删除升级保存地址所在的flash空间
	eat_flash_erase((void *)(APP_DATA_STORAGE_BASE), Lappupdate.datalen);
	//将升级程序写入到flash空间区域，起始地址为APP_DATA_STORAGE_BASE
	eat_flash_write((void *)(APP_DATA_STORAGE_BASE), Lappupdate.data, Lappupdate.datalen);
	//升级程序
	eat_update_app((void *)(APP_DATA_RUN_BASE),(void *)(APP_DATA_STORAGE_BASE),Lappupdate.datalen, EAT_PIN_NUM,EAT_PIN_NUM,EAT_FALSE);
	
	
	return 0;
}


extern u8 *usr_mallocEx(void);
u8 update_start(u32 datalen, u8 type, u8 *name){
	
	if(NULL == name || strlen((s8 *)name) > 31){
		user_debug("i:update-start input error");
		return 1;
	}
	if(datalen > BIG_MEM_MAX){//下载的文件最大只允许150K
		user_debug("i:update-file too long:%d",datalen);
		return 1;
	}
	Lappupdate.datalen = datalen;
	Lappupdate.data = bigmem_get(2);//eat_mem_alloc(Lappupdate.datalen + 1);
	if(NULL == Lappupdate.data)
	{
		user_debug("i:update-start:malloc error[%d]", datalen + 1);
		return 1;
	}
	strcpy((s8 *)Lappupdate.name, (s8 *)name);
	Lappupdate.dataindex = 0;
	Lappupdate.cs = 0;
	Lappupdate.flag = type;//开始升级
	user_debug("i:updatestart[%d-%s]",type, name);
	return 0;
}

void update_end(void){
	Lappupdate.flag = 0;
	Lappupdate.datalen = 0;
	Lappupdate.dataindex = 0;
	Lappupdate.frameindex = 0;
}

u8 update_datain(u8 *data, u16 datalen){
	u16 dataindex;
	u8 *dataptr;
	
	if(Lappupdate.flag < 1)return 1;
	if(NULL == data || 0 == datalen){
		user_debug("i:update_datain error:datalen =%d",datalen);
		return 2;
	}
	dataptr = Lappupdate.data;
	for(dataindex = 0; dataindex < datalen; dataindex ++){
		*(dataptr + Lappupdate.dataindex) = *(data + dataindex);
		Lappupdate.dataindex ++;
		if(Lappupdate.dataindex > Lappupdate.datalen){
			user_debug("i:update_datain error:[%d-%d]",Lappupdate.dataindex,Lappupdate.datalen);
			Lappupdate.flag = 0;
			return 3;
		}
	}
	return 0;
}

u8 update_datainEx(u8 *data, u16 datalen, u32 findex){
	u16 dataindex;
	u8 *dataptr;
	
	if(Lappupdate.flag < 1)return 1;
	if(NULL == data || 0 == datalen){
		user_debug("i:update_datain error:datalen =%d",datalen);
		return 2;
	}
	if(findex != Lappupdate.frameindex){
		user_debug("i:update_datain frameindex[%d-%d]",findex, Lappupdate.frameindex);
		if(findex < Lappupdate.frameindex)return 0;//该帧已经接收
		else return 2;
	}
	dataptr = Lappupdate.data;
	for(dataindex = 0; dataindex < datalen; dataindex ++){
		*(dataptr + Lappupdate.dataindex) = *(data + dataindex);
		Lappupdate.dataindex ++;
		if(Lappupdate.dataindex > Lappupdate.datalen){
			user_debug("i:update_datain error:[%d-%d]",Lappupdate.dataindex,Lappupdate.datalen);
			Lappupdate.flag = 0;
			Lappupdate.frameindex = 0;
			return 3;
		}
	}
	user_debug("i:bags=%d-,len=%d",Lappupdate.frameindex, Lappupdate.dataindex);
	Lappupdate.frameindex ++;
	return 0;
}

u8 update_cs(u32 cs){
	u32 dataindex;
	u8 *dataptr;
	u32 cs1;
	
	if(Lappupdate.dataindex != Lappupdate.datalen){
		Lappupdate.flag = 0;
		user_debug("i:update-cs len error:%d-%d", Lappupdate.dataindex,Lappupdate.datalen);
		return 1;
	}
	if(Lappupdate.flag < 1){
		user_debug("i:update-cs cs error");
		return 2;
	}
	cs1 = 0;
	dataptr = Lappupdate.data;
	for(dataindex = 0; dataindex < Lappupdate.datalen; dataindex ++){
		cs1 += *(dataptr + dataindex);
	}
	if(cs != cs1){
		user_debug("i:update-cs cs error:%d-%d", cs,cs1);
		Lappupdate.flag = 0;
		return 3;
	}
	Lappupdate.cs = cs;
	return 0;
}

extern unsigned char paramcodeLink(unsigned char *data, unsigned char datalen, unsigned char *id);
/*License数据从M2M下载到OBD中
**************************************************/
u8 update_obdlicense(void){
	u8 u8t1,u8t2;
	u8 senddata[84];
	u8 loopnum;
	u32 overtime;
	u8 cmd1,cmd2;
	u16 datalen;
	u8 *data;
	u8 keys[20],LmucId[20];
	
	if(Lappupdate.flag != UPDATE_OBD_LICENSE || Lappupdate.datalen != 1024){
		user_debug("i:update_obdlicense type error");
		update_end();
		return 1;
	}
	loopnum = 0;
	
	//eat_sleep(20*1000);//睡眠20S
	
	obd_Rx_bufclr();//临时处理方式 测试发现当进入升级等待时接收缓冲区接近满状态 这时可能导致系统死机 由于两个任务同时操作了接收缓冲区
	//第一步：握手
	loopnum = 0;
_step1:
  user_debug("i:update_obdlicense..");
	senddata[0] = 0x01;
  senddata[1] = 0x01;
  obd_write(senddata, 2);
  overtime = 0;
  while(1){
  	cmd1 = 0;
  	if(0x00 == obd_data_read(&cmd1, &cmd2, &data, &datalen)){
  		if(0x41 == cmd1 && 0x01 == *(data + 5) && datalen > 32)break;
  		else if(0x41 == cmd1 && 0x00 == *(data + 5))return 0;//不需要升级
  	}
  	overtime ++;
  	if(overtime > 100){//2000ms超时
  		if(loopnum > 3){
  			user_debug("i:update_obdlicense step1 overtime");
  			update_end();
  			return 2;
  		}
  		else{
  		   loopnum ++;
  		   goto _step1;
  		  }
  	}
  	eat_sleep(5);
  }
  user_infor("e:update_obdlicense step1 done");
  for(u8t1 = 0; u8t1 < 16; u8t1 ++)LmucId[u8t1] = *(data + 6 + u8t1);
  for(u8t1 = 0; u8t1 < 16; u8t1 ++)keys[u8t1] = *(data + 22 + u8t1);
  user_infor("e:update_obdlicense step1 done1");
  paramcodeLink(keys, 16, LmucId);
 
  loopnum = 0;
 _step1_1:
  user_debug("i:update_obdlicense step1.1");
  senddata[0] = 0x01;
  senddata[1] = 0x02;
  for(u8t1 = 0; u8t1 < 16; u8t1 ++)senddata[2 + u8t1] = keys[u8t1];
  obd_write(senddata, 18);
  overtime = 0;
  while(1){
  	cmd1 = 0;
  	if(0x00 == obd_data_read(&cmd1, &cmd2, &data, &datalen)){
  		if(0x41 == cmd1)break;
  		else if(0x7f == cmd1)return 2;
  	}
  	overtime ++;
  	if(overtime > 100){//500ms超时
  		  if(loopnum > 3){
  			    user_debug("i:update_obdlicense step1.1 overtime");
  			    update_end();
  			    return 2;
  		  }
  		  else{
  		   loopnum ++;
  		   goto _step1_1;
  		  }
  	}
  	eat_sleep(5);
  }
  
  //第二步:数据下载
  user_debug("i:update_obdlicense step2");
	for(u8t1 = 0; u8t1 < 16; u8t1 ++){
		loopnum = 0;
_Loop:
    senddata[0] = 0x02;
    senddata[1] = u8t1;
    for(u8t2 = 0; u8t2 < 64; u8t2 ++)senddata[2 + u8t2] = *(Lappupdate.data + u8t1 * 64 + u8t2);
    obd_write(senddata, 66);
    overtime = 0;
    while(1){
    	cmd1 = 0;
    	if(0x00 == obd_data_read(&cmd1, &cmd2, &data, &datalen)){
    		if(0x42 == cmd1){
    			if(0x00 == cmd2)break;
    			else if(0x01 == cmd2)break;
    			else goto _Loop;
    		}
    		else if(0x7f == cmd1)goto _Loop;
    	}
    	overtime ++;
    	if(overtime > 100){//500ms超时
    		if(loopnum > 3){
    			user_debug("i:update_obdlicense overtime[%d]", u8t1);
    			update_end();
    			return 2;
    		}
    		else{
    		   loopnum ++;
    		   goto _Loop;
    		  }
    	}
    	eat_sleep(5);
    }
	}
	db_update_save(Lappupdate.flag, Lappupdate.name, Lappupdate.datalen, Lappupdate.cs);
	update_end();
	user_debug("i:update_obdlicense OK");
	return 0;
}

/*OBD协议栈升级
****************************************/
u8 update_obdapp(void){
	u8 u8t1,u8t2,dataindex,datalen;
	u8 senddata[128];
	u8 loopnum,overtime;
	u8 cmd1,cmd2;
	u16 datalen1,framenum,frameindex;
	u32 sendindex,sendindexlog;
	u8 *data,errornum,errornum1;
	
	if(Lappupdate.flag != UPDATE_OBD_APP){
		user_debug("i:update_obdapp type error");
		update_end();
		return 1;
	}
	eat_sleep(2000);//睡眠2S
	user_debug("i:update_obdapp..");
	errornum = 0;
	//第一步：0x84 0x07
_step1:
  if(errornum > 6){
  	update_end();
  	user_debug("i:update_obdapp too much error");
		return 1;
  }
  user_debug("i:update_obdapp step1");
	senddata[0] = 0x84;
	senddata[1] = 0x07;
	senddata[2] = UPDATE_OBD_APP;
	obd_write(senddata, 3);
	overtime = 0;
	while(1){
		cmd1 = 0;
		if(0x00 == obd_data_read(&cmd1, &cmd2, &data, &datalen1)){
			if(0x84 == cmd1)break;
			else if(0x7f == cmd1){
			   errornum ++;
			   goto _step1;
		  }
		}
		overtime ++;
		if(overtime >= 200){//超时1S
			errornum ++;
			goto _step1;
		}
		eat_sleep(5);
	}
	eat_sleep(2000);
	
	//第二步84 02 发送文件名 大小
_step2:
  user_debug("i:update_obdapp step2[%d name=%s]", Lappupdate.datalen,Lappupdate.name);
  memset((s8 *)senddata, 0, 128);
	senddata[0] = 0x84;
	senddata[1] = 0x02;
	strcpy((s8 *)&senddata[2], (s8 *)Lappupdate.name);
	u8t1 = strlen((s8 *)Lappupdate.name);
	senddata[3 + u8t1 ++] = (Lappupdate.datalen >> 24) & 0x00ff;
	senddata[3 + u8t1 ++] = (Lappupdate.datalen >> 16) & 0x00ff;
	senddata[3 + u8t1 ++] = (Lappupdate.datalen >> 8) & 0x00ff;
	senddata[3 + u8t1 ++] = (Lappupdate.datalen >> 0) & 0x00ff;
	obd_write(senddata, 3 + u8t1);
	overtime = 0;
	while(1){
		cmd1 = 0;
		if(0x00 == obd_data_read(&cmd1, &cmd2, &data, &datalen1)){
			if(0x84 == cmd1)break;
			else if(0x7f == cmd1){
				 errornum ++;
				 user_debug("i:update_obdapp step2 7f error");
			   goto _step1;
		  }
		}
		overtime ++;
		if(overtime >= 600){//超时3S  需要擦除FLASH需要比较长的时间
			errornum ++;
			user_debug("i:step2 overtime");
			goto _step1;
		}
		eat_sleep(5);
	}
	//第三步 0x84 0x03 发送数据
_step3:	
  user_debug("i:update_obdapp step3");
	datalen = 100;//默认每次发送100个字节
	framenum = Lappupdate.datalen / 100;
	if(Lappupdate.datalen % 100 != 0)framenum ++;
	sendindex = 0;
	sendindexlog = sendindex;
	for(frameindex = 0; frameindex < framenum; frameindex ++){
		errornum1 = 0;
_resend:
     if(errornum1 > 3){
     	  user_debug("i:update_obdapp step3 too much error");
			  update_end();
			  return 2;
     }
     memset(senddata, 0, 128);
     dataindex = 0;
     senddata[dataindex ++] = 0x84;
     senddata[dataindex ++] = 0x03;
     senddata[dataindex ++] = (frameindex >> 8) & 0x00ff;
     senddata[dataindex ++] = (frameindex >> 0) & 0x00ff;
     senddata[dataindex ++] = (datalen >> 0) & 0x00ff;
     for(dataindex = 0; dataindex < datalen; dataindex ++){
     	 senddata[dataindex + 5] = *(Lappupdate.data + sendindex);
     	 sendindex ++;
     }
     obd_write(senddata, 5 + dataindex);
	   overtime = 0;
	   while(1){
	   	    cmd1 = 0;
		      if(0x00 == obd_data_read(&cmd1, &cmd2, &data, &datalen1)){
			        if(0x84 == cmd1 && 0x03 == cmd2){
			        	if(0 == *(data + 6)){
			        	   if(frameindex + 1 == (*(data + 7)) * 0x100 + *(data + 8))break;
			        	   else{
			        	       sendindex = sendindexlog;
			        	       errornum1 ++;
			        	       goto _resend;
			        	   }
			          }
			          else{
			             sendindex = sendindexlog;
			             errornum1 ++;
			        	   goto _resend;
			          }
			        }
			        else if(0x7f == cmd1){
			        	  errornum ++;
			        	  goto _step1;
			        }
		      }
		      overtime ++;
		      if(overtime >= 200){//超时1S
			        sendindex = sendindexlog;
			        errornum1 ++;
			        goto _resend;
		      }
		      eat_sleep(5);
	    }
	    sendindexlog = sendindex;
	    if(sendindex + 100 < Lappupdate.datalen)datalen = 100;
	    else datalen = Lappupdate.datalen - sendindex;
	}
	
	//第四步 发送校验字 0x84 0x04
_step4:
  user_debug("i:update_obdapp step4[%d]",Lappupdate.cs);
	dataindex = 0;
	senddata[dataindex ++] = 0x84;
	senddata[dataindex ++] = 0x04;
  senddata[dataindex ++] = (Lappupdate.cs >> 24) & 0x00ff;
  senddata[dataindex ++] = (Lappupdate.cs >> 16) & 0x00ff;
  senddata[dataindex ++] = (Lappupdate.cs >> 8) & 0x00ff;
  senddata[dataindex ++] = (Lappupdate.cs >> 0) & 0x00ff;
  obd_write(senddata, 6);
	overtime = 0;
	while(1){
	    cmd1 = 0;
	    if(0x00 == obd_data_read(&cmd1, &cmd2, &data, &datalen1)){
	    	  if(0x84 == cmd1)break;
	    	  else if(0x7f == cmd1){
	    	  	errornum ++;
	    	  	goto _step1;
	    	  }
	     }
	    overtime ++;
	    if(overtime >= 200){//超时1S
			    user_debug("i:update_obdapp step4 error");
			    update_end();
			    return 3;
		  }
		  eat_sleep(5);
	}
  //第五步 重新启动
_step5:
  user_debug("i:update_obdapp step5");
  dataindex = 0;
	senddata[dataindex ++] = 0x84;
	senddata[dataindex ++] = 0x05;
	obd_write(senddata, 2);
	
	db_update_save(Lappupdate.flag, Lappupdate.name, Lappupdate.datalen, Lappupdate.cs);
	update_end();
	return 0;
}



/*语音文件升级
****************************************/
u8 update_obdmusic(void){
	u8 u8t1,u8t2,dataindex,datalen;
	u8 senddata[128];
	u8 loopnum,overtime;
	u8 cmd1,cmd2;
	u16 datalen1,framenum,frameindex;
	u32 sendindex,sendindexlog;
	u8 *data;
	
	
	//第一步84 07 升级请求
	senddata[0] = 0x84;
	senddata[1] = 0x07;
	senddata[2] = UPDATE_MUSIC;
	obd_write(senddata, 3);
	overtime = 0;
	while(1){
		cmd1 = 0;
		if(0x00 == obd_data_read(&cmd1, &cmd2, &data, &datalen1)){
			if(0x84 == cmd1 && 0x07 == cmd2)break;
		}
		overtime ++;
		if(overtime >= 200){//超时1S
			user_debug("i:update_obdmusic step1 error");
			return 1;
		}
		eat_sleep(5);
	}
	//第二步84 02 发送文件名 大小
	senddata[0] = 0x84;
	senddata[1] = 0x02;
	strcpy((s8 *)&senddata[2], (s8 *)Lappupdate.name);
	u8t1 = strlen((s8 *)Lappupdate.name);
	senddata[2 + u8t1 ++] = 0;
	senddata[2 + u8t1 ++] = (Lappupdate.datalen >> 24) & 0x00ff;
	senddata[2 + u8t1 ++] = (Lappupdate.datalen >> 16) & 0x00ff;
	senddata[2 + u8t1 ++] = (Lappupdate.datalen >> 8) & 0x00ff;
	senddata[2 + u8t1 ++] = (Lappupdate.datalen >> 0) & 0x00ff;
	obd_write(senddata, 2 + u8t1);
	overtime = 0;
	while(1){
		cmd1 = 0;
		if(0x00 == obd_data_read(&cmd1, &cmd2, &data, &datalen1)){
			if(0x84 == cmd1 && 0x02 == cmd2)break;
		}
		overtime ++;
		if(overtime >= 200){//超时1S
			user_debug("i:update_obdmusic step1 error");
			return 1;
		}
		eat_sleep(5);
	}
	//第三步 0x83 0x03 发送数据
	datalen = 100;//默认每次发送100个字节
	framenum = Lappupdate.datalen / 100;
	if(Lappupdate.datalen % 100 != 0)framenum ++;
	sendindex = 0;
	sendindexlog = sendindex;
	for(frameindex = 0; frameindex < framenum; frameindex ++){
_resend:
     memset(senddata, 0, 128);
     dataindex = 0;
     senddata[dataindex ++] = 0x84;
     senddata[dataindex ++] = 0x03;
     //senddata[dataindex ++] = (frameindex >> 8) & 0x00ff;
     senddata[dataindex ++] = (frameindex >> 0) & 0x00ff;
     senddata[dataindex ++] = (datalen >> 8) & 0x00ff;
     senddata[dataindex ++] = (datalen >> 0) & 0x00ff;
     for(dataindex = 0; dataindex < datalen; dataindex ++){
     	 senddata[dataindex + 5] = *(Lappupdate.data + sendindex);
     	 sendindex ++;
     }
     obd_write(senddata, 5 + dataindex);
	   overtime = 0;
	   while(1){
	   	    cmd1 = 0;
		      if(0x00 == obd_data_read(&cmd1, &cmd2, &data, &datalen1)){
			        if(0x84 == cmd1 && 0x03 == cmd2){
			        	if(0 == *(data + 6))break;
			        	else if(0x01 == *(data + 6)){
			        		sendindex = sendindexlog;
			        		goto _resend;
			          }
			          else{
			        	   user_debug("i:update_obdapp step2 error");
			             return 2;
			          }
			        }
			        else if(0x7f == cmd1){
			        		sendindex = sendindexlog;
			        		goto _resend;
			        }
		      }
		      overtime ++;
		      if(overtime >= 200){//超时1S
			        user_debug("i:update_obdapp step2-2 error");
			        return 2;
		      }
		      eat_sleep(5);
	    }
	    sendindexlog = sendindex;
	    if(sendindex + 100 < Lappupdate.datalen)datalen = 100;
	    else datalen = Lappupdate.datalen - sendindex;
	}
	
	//第四步 发送校验字 0x84 0x04
	dataindex = 0;
	senddata[dataindex ++] = 0x84;
	senddata[dataindex ++] = 0x04;
  senddata[dataindex ++] = (Lappupdate.cs >> 24) & 0x00ff;
  senddata[dataindex ++] = (Lappupdate.cs >> 16) & 0x00ff;
  senddata[dataindex ++] = (Lappupdate.cs >> 8) & 0x00ff;
  senddata[dataindex ++] = (Lappupdate.cs >> 0) & 0x00ff;
  obd_write(senddata, 6);
	overtime = 0;
	while(1){
	    cmd1 = 0;
	    if(0x00 == obd_data_read(&cmd1, &cmd2, &data, &datalen1)){
	    	  if(0x84 == cmd1 && 0x4 == cmd2)break;
	     }
	    overtime ++;
	    if(overtime >= 200){//超时1S
			    user_debug("i:update_obdapp step3 error");
			    return 3;
		  }
		  eat_sleep(5);
	}
	
	db_update_save(Lappupdate.flag, Lappupdate.name, Lappupdate.datalen, Lappupdate.cs);
	update_end();
	return 0;
}

