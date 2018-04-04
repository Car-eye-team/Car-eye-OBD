/*
*
*系统参数的保存于读取
*******************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rt_misc.h>

#include "eat_modem.h"
#include "eat_interface.h"
#include "eat_fs_errcode.h" 
#include "eat_fs_type.h" 
#include "eat_fs.h"
#include "definedata.h"
#include "db.h"
#include "UartTask.h"
#include "BigMem.h"

static u8 fs_buff_temp[1024];
#define update_file L"C:\\update"
/*文件读入，读入数据保存到fs_buff_temp
*
*********************************************/
u32 db_update_file_read(void){
	s32 fpr,seekRet;
	u32 filesize,readLen;
	eat_fs_error_enum fserror;
	

	fpr =  eat_fs_Open(update_file, FS_READ_ONLY);
	if(fpr<EAT_FS_NO_ERROR){
		fpr =  eat_fs_Open(update_file,FS_CREATE);
		if(fpr<EAT_FS_NO_ERROR){
			user_debug("i:Open File Fail,and Return Error is %d",fpr);
			return 0;
		}
		eat_fs_Close(fpr);
		return 0;    
	}  
	
	fserror = (eat_fs_error_enum)eat_fs_GetFileSize(fpr,&filesize);
	if(0 == filesize || filesize >= 1024){
		eat_fs_Close(fpr);
		return 2;
	}
	if(EAT_FS_NO_ERROR != fserror)
	{
		user_debug("i:Get File Size Fail,and Return Error is %d",fserror);
		eat_fs_Close(fpr);
		return 0;
	}
	
	seekRet = eat_fs_Seek(fpr,0,EAT_FS_FILE_BEGIN);
	if(0>seekRet)
	{
		user_debug("i:eat_fs_Seek():Seek File Pointer Fail");
		eat_fs_Close(fpr);
		return 0;
	}
	
	fserror = (eat_fs_error_enum)eat_fs_Read(fpr,fs_buff_temp,filesize,&readLen);
	if(EAT_FS_NO_ERROR != fserror ||filesize !=readLen)
	{	
		user_debug("i:eat_fs_Read():Read File Fail,and Return Error is %d,Readlen is %d",fserror,readLen);
		eat_fs_Close(fpr);
		return 0;
	}
	eat_fs_Close(fpr);
	return readLen;
}


#define updateinfor_file L"C:\\update"
u8 db_swver_save(u8 *infor){
	eat_fs_error_enum fserror;
	s32 fpr,seekRet,filesize;
	
	if(NULL == infor)return 1;
	fpr = eat_fs_Open(updateinfor_file, FS_READ_WRITE | FS_CREATE);
	if(fpr<EAT_FS_NO_ERROR){
		user_debug("i:db_swver_save fileopen error");
		return 1;
	}
	eat_fs_Seek(fpr,0,EAT_FS_FILE_BEGIN);
	fserror = eat_fs_Write(fpr, infor + 1, *(infor + 0), &filesize);
	if(EAT_FS_NO_ERROR!= fserror|| *(infor + 0) != filesize){
		user_debug("i:db_swver_save data write error");
		eat_fs_Close(fpr);
		return 1;
	}
	else{
	  fserror = eat_fs_Commit(fpr);
	  if(EAT_FS_NO_ERROR != fserror ){
	  	user_debug("i:db_swver_save commit error");
	  	eat_fs_Close(fpr);
		  return 1;
	  }
	  eat_fs_Close(fpr);
	  user_infor("e:db_swver_save OK");
		return 0;
	}
}
_update Lupdate;
/*升级信息保存
*M2M需要保存M2M以及OBD模块所有升级文件信息，格式如下：
*type,size,cs,name  //一行为一条信息，回车结束，ASCII保存，每个信息以','分隔
*******************************************************/
u8 db_update_init(void){
	u32 readLen,readindex;
	u8 step;
	u8 u8t1;
	u32 u32t1;
	
	Lupdate.filenum = 0;
	readLen = db_update_file_read();
	if(0 == readLen){
		return 1;
	}
	for(step = 0; step < UPDATE_FILE_MAX; step ++)
	{
		Lupdate.file[step].type = 0;
	}
	step = 0;
	readindex = 0;
	u8t1 = 0;
	u32t1 = 0;
	if(readLen)
	{//文件解析
		while(1)
		{
			if(',' == fs_buff_temp[readindex])
			{
				if(0 == step)Lupdate.file[Lupdate.filenum].type = u32t1 & 0x00ff;
				else if(1 == step)Lupdate.file[Lupdate.filenum].size = u32t1;
				else if(2 == step)Lupdate.file[Lupdate.filenum].cs = u32t1;
				step ++;
				u32t1 = 0;
				readindex ++;
				if(readindex >= readLen)
				{
					user_debug("i:update_init error0:%d",Lupdate.filenum);
			    		Lupdate.filenum = 0;
			    		return 2;
				}
				continue;
			}
			else if(0x0d == fs_buff_temp[readindex] || 0x0a == fs_buff_temp[readindex])
			{
				if(3 == step)Lupdate.filenum ++;
				while(1)
				{
					if(0x0d == fs_buff_temp[readindex] || 0x0a == fs_buff_temp[readindex]);
					else break;
					readindex ++;
					if(readindex >= readLen)return 0;
				}
				step = 0;
				continue;
			}
			else
			{
			    if(3 == step)
			    {
			    		memset(Lupdate.file[Lupdate.filenum].name, 0, 32);
			    		for(u8t1 = 0; u8t1 < 32; u8t1 ++)
					{
			    			if(0x0d == fs_buff_temp[readindex] || 0x0a == fs_buff_temp[readindex])
						{
			    				user_infor("e:Read-file[%d]%s", Lupdate.filenum,Lupdate.file[Lupdate.filenum].name);
			    				break;
			    			}
			    			Lupdate.file[Lupdate.filenum].name[u8t1] = fs_buff_temp[readindex];
			    			readindex ++;
			    			if(readindex >= readLen)
						{//文件异常
			    				user_debug("i:update_init error1:%d",Lupdate.filenum);
			    				Lupdate.filenum = 0;
			    				return 2;
			    			}
			    		}
			    }
			    else
			    {
			        if(fs_buff_temp[readindex] >= '0' && fs_buff_temp[readindex] <= '9')u32t1 = (u32t1 << 4) + fs_buff_temp[readindex] - '0';
			        else if(fs_buff_temp[readindex] >= 'A' && fs_buff_temp[readindex] <= 'F')u32t1 = (u32t1 << 4) + fs_buff_temp[readindex] - 'A' + 0x0a;
			        else
				 {
			            user_debug("i:update_init error2:%d",Lupdate.filenum);
			            Lupdate.filenum = 0;
			            return 2;
			        }
			        readindex ++;
			        if(readindex >= readLen)
				 {//文件异常
			    			user_debug("i:update_init error3:%d",Lupdate.filenum);
			    			Lupdate.filenum = 0;
			    			return 2;
			    	}
			  }
		  }
	    }
	}
	return 1;
}

/*根据文件类型获取文件名
*************************************/
u8 *db_update_fileget(u8 type){
	u8 u8index;
	for(u8index = 0; u8index < UPDATE_FILE_MAX; u8index ++){
		if(type == Lupdate.file[u8index].type)return Lupdate.file[u8index].name;
	}
	return NULL;
}

/*判断升级过程中当前版本与待升级的版本，如果待升级的版本比当前版本新则升级
*否则不升级
*返回：0 = 允许升级 
       否则不允许升级
*/
u8 db_update_vercheck(u16 ver, u8 type){
	u8 *vers,data;
	u16 vercur;
	
	vers = db_update_fileget(type);
	if(NULL == vers){
		user_debug("i:update[%d] ver is NULL",type);
		return 0;
	}
	user_debug("i:update ver:%d-%s", ver,vers);
	data = *(vers);
	if(data >= '0' && data <= '9')vercur = *(vers) - '0';
	else return 0;
	data = *(vers + 1);
	if(data >= '0' && data <= '9')vercur = (vercur << 4) + (*(vers + 1) - '0');
	else return 0;
	data = *(vers + 2);
	if(data >= '0' && data <= '9')vercur = (vercur << 4) + (*(vers + 2) - '0');
	else return 0;
	data = *(vers + 3);
	if(data >= '0' && data <= '9')
	if(data >= '0' && data <= '9')vercur = (vercur << 4) + (*(vers + 3) - '0');
	else return 0;
	if(ver > vercur)return 0;
	return 1;
}

u8 db_init(void)
{
	u8 u8result;
	u8 *ObdVer=NULL;
	u8 *M2mVer=NULL;
	
	u8result = db_update_init();
	u8result += db_svr_init();
	db_obd_init();
	db_fileinit();
	db_gps_init();

       /*add by lilei-2016-04-08 ver show*/
	ObdVer=db_update_fileget(0x0c);
	M2mVer = db_update_fileget(0x10);
	if(ObdVer!=NULL)
	{
		user_debug("\r\ni:ObdVer=%s",ObdVer);

	}
	else
	{
		user_debug("i:ObdVer=NULL\r\n",ObdVer);

	}
	if(M2mVer!=NULL)
	{
		user_debug("i:M2mVer=%s\r\n",M2mVer);

	}
       
	return u8result;
}

/*数据保存 保存的内容已经文件名均由主调函数决定
*/
u8 db_save(u8 *file, u32 filesize, u8 *filename){
	char filepath[128];
	wchar_t wfilepath[128];
	int i32t1,i32t2;
	s32 fpr;
	u32 filesizeex;
	eat_fs_error_enum fserror;
	
	if(NULL == file || NULL == filename)return 1;
	memset(filepath, 0, 128);
	sprintf(filepath, "C:\\%s", (s8 *)filename);
	for(i32t2 = 0; i32t2 < 128; i32t2 ++)wfilepath[i32t2] = 0;
	mbstowcs(wfilepath,filepath, strlen(filepath));
	
	fpr =  eat_fs_Open(wfilepath, FS_CREATE | FS_READ_WRITE);
	if(fpr < EAT_FS_NO_ERROR){
		 user_debug("i:db_save[%d] fileopen error",fpr);
		 return 4;
	}
	eat_fs_Seek(fpr,0,EAT_FS_FILE_BEGIN);
	fserror = eat_fs_Write(fpr, file, filesize, &filesizeex);
	if(EAT_FS_NO_ERROR!= fserror||filesize != filesizeex){
		user_debug("i:db_save data write error");
		eat_fs_Close(fpr);
		return 5;
	}
	else{
	  fserror = eat_fs_Commit(fpr);
	  if(EAT_FS_NO_ERROR != fserror ){
	  	user_debug("i:db_save commit error");
	  	eat_fs_Close(fpr);
		  return 6;
	  }
	  eat_fs_Close(fpr);
	  user_infor("e:db_save OK");
		return 0;
	}
	return 0;
}


u8 db_update_save(u8 type, u8 *name, u32 size, u32 cs){
	u8 u8t1;
	u8 datasavebuf[265],dataindex;
	s32 fpr,seekRet;
	u32 filesize,readLen;
	eat_fs_error_enum fserror;
	
	if(type < 1)return 1;
	if(NULL == name || strlen((s8 *)name) > 30)return 2;
	for(u8t1 = 0; u8t1 < Lupdate.filenum; u8t1 ++)
	{
		if(Lupdate.file[u8t1].type == type)break;
	}
	if(u8t1 >= Lupdate.filenum)
	{//当前文件没有记录
	   	if(Lupdate.filenum >= UPDATE_FILE_MAX)
		{
	    		user_debug("i:db_update_save error");
	   		return 3;//异常
	   	}
	   	u8t1 = Lupdate.filenum;
	   	Lupdate.filenum ++;
	}
	Lupdate.file[u8t1].type = type;
	strcpy((s8 *)Lupdate.file[u8t1].name, name);
	Lupdate.file[u8t1].size = size;
	Lupdate.file[u8t1].cs = cs;
	
	dataindex = 0;
	memset(datasavebuf, 0, 256);
	for(u8t1 =0; u8t1 < Lupdate.filenum; u8t1 ++){
		strcat((s8 *)&datasavebuf[dataindex], u8Str(Lupdate.file[u8t1].type));
		dataindex += 2;
		strcat((s8 *)&datasavebuf[dataindex], ",");
		dataindex += 1;
		strcat((s8 *)&datasavebuf[dataindex], u32Str(Lupdate.file[u8t1].size));
		dataindex += 8;
		strcat((s8 *)&datasavebuf[dataindex], ",");
		dataindex += 1;
		strcat((s8 *)&datasavebuf[dataindex], u32Str(Lupdate.file[u8t1].cs));
		dataindex += 8;
		strcat((s8 *)&datasavebuf[dataindex], ",");
		dataindex += 1;
		strcat((s8 *)&datasavebuf[dataindex], (s8 *)Lupdate.file[u8t1].name);
		dataindex += strlen((s8 *)Lupdate.file[u8t1].name);
		datasavebuf[dataindex ++] = 0x0d;
		datasavebuf[dataindex ++] = 0x0a;
	}
	
	fpr =  eat_fs_Open(update_file, FS_CREATE | FS_READ_WRITE);
	if(fpr < EAT_FS_NO_ERROR){
		 user_debug("i:db_update_save[%d] fileopen error",fpr);
		 return 4;
	}
	eat_fs_Seek(fpr,0,EAT_FS_FILE_BEGIN);
	fserror = eat_fs_Write(fpr, datasavebuf, dataindex, &filesize);
	if(EAT_FS_NO_ERROR!= fserror||dataindex != filesize){
		user_debug("i:db_update_save data write error");
		eat_fs_Close(fpr);
		return 5;
	}
	else{
	  fserror = eat_fs_Commit(fpr);
	  if(EAT_FS_NO_ERROR != fserror ){
	  	user_debug("i:db_update_save commit error");
	  	eat_fs_Close(fpr);
		  return 6;
	  }
	  eat_fs_Close(fpr);
	  user_infor("e:db_update_save OK");
		return 0;
	}
	return 0;
}


/****************************************************************************************************************/
//M2M信息数据处理
static _SVR Lsvrinfor;
#define SVR_file L"C:\\svr.txt"

u32 db_svr_file_read(void){
	s32 fpr,seekRet;
	u32 filesize,readLen;
	eat_fs_error_enum fserror;
	

	fpr =  eat_fs_Open(SVR_file, FS_READ_ONLY);
	if(fpr < EAT_FS_NO_ERROR)
	{
		fpr =  eat_fs_Open(SVR_file,FS_CREATE);
		if(fpr<EAT_FS_NO_ERROR)
		{
			user_debug("i:Open SVR_File Fail,and Return Error is %d",fpr);
			return 0;
		}
		eat_fs_Close(fpr);
		return 0;    
	}  
	
	fserror = (eat_fs_error_enum)eat_fs_GetFileSize(fpr,&filesize);
	if(0 == filesize || filesize >= 1024)
	{
		eat_fs_Close(fpr);
		return 2;
	}
	if(EAT_FS_NO_ERROR != fserror)
	{
		user_debug("i:Get SVR_File Size Fail,and Return Error is %d",fserror);
		eat_fs_Close(fpr);
		return 0;
	}
	
	seekRet = eat_fs_Seek(fpr,0,EAT_FS_FILE_BEGIN);
	if(0>seekRet)
	{
		user_debug("i:eat_fs_Seek():Seek File Pointer Fail");
		eat_fs_Close(fpr);
		return 0;
	}
	
	fserror = (eat_fs_error_enum)eat_fs_Read(fpr,fs_buff_temp,filesize,&readLen);
	if(EAT_FS_NO_ERROR != fserror ||filesize !=readLen)
	{	
		user_debug("i:eat_fs_Read():Read File Fail,and Return Error is %d,Readlen is %d",fserror,readLen);
		eat_fs_Close(fpr);
		return 0;
	}
	eat_fs_Close(fpr);
	return readLen;
}
/*保存服务器相关信息，如：IP、端口如下：
*IP:XXX
*PORT:XXX
*STEL:XXXX
*CYCL:XXXXX
*****************************************************/
u8 db_svr_init(void){
	u32 filesize,fileindex;
	u8 u8t1;
	u8 type;


	filesize = db_svr_file_read();
	if(filesize < 12)
	{
		user_infor("e:db_svr_init to default");
		if(db_svr_default() != 0)return 1;
	}
	fileindex = 0;
	Lsvrinfor.backcircle = 0;//默认为0
	memset(Lsvrinfor.svraddr, 0, 128);
	Lsvrinfor.port = 0;
	memset(Lsvrinfor.svraddr1, 0, 128);
	Lsvrinfor.port1 = 0;
	Lsvrinfor.media = 1;//多媒体使能
	memset(Lsvrinfor.svrtell_t, 0, 12);
	memset(Lsvrinfor.svrtell_r, 0, 12);
	memset(Lsvrinfor.telx[0], 0, 12);
	memset(Lsvrinfor.telx[1], 0, 12);
	memset(Lsvrinfor.telx[2], 0, 12);
	memset(Lsvrinfor.telx[3], 0, 12);
	memset(Lsvrinfor.telx[4], 0, 12);
	memset(Lsvrinfor.apn, 0, 16);
	memset(Lsvrinfor.ttell, 0, 32);

	
	while(1)
	{
		if('I' == fs_buff_temp[fileindex] && 'P' == fs_buff_temp[fileindex + 1])
		{//IP地址
			if('0' == fs_buff_temp[fileindex + 2])type = 0;
			else if('1' == fs_buff_temp[fileindex + 2])type = 1;
			else
			{//数据异常
				fileindex += 4;
				while(1)
				{
					if(0x0d == fs_buff_temp[fileindex] || 0x0a == fs_buff_temp[fileindex])break;
					fileindex ++;
				}
				continue;
			}
			fileindex += 4;
			u8t1 = 0;
			while(1)
			{
				if(0x0d == fs_buff_temp[fileindex] || 0x0a == fs_buff_temp[fileindex])break;
				if(0 == type)Lsvrinfor.svraddr[u8t1] = fs_buff_temp[fileindex];
				else Lsvrinfor.svraddr1[u8t1] = fs_buff_temp[fileindex];
				fileindex ++;
				u8t1 ++;
				if(u8t1 >= 31 || fileindex >= filesize)
				{
					user_debug("i:svr_file_init error1");
					return 2;
				}
			}
		}
		else if('P' == fs_buff_temp[fileindex] && 'O' == fs_buff_temp[fileindex + 1] && 'R' == fs_buff_temp[fileindex + 2] && 'T' == fs_buff_temp[fileindex + 3])
		{//服务器地址
	    		if('0' == fs_buff_temp[fileindex + 4])type = 0;
			else if('1' == fs_buff_temp[fileindex + 4])type = 1;
			else
			{//数据异常
				fileindex += 6;
				while(1)
				{
					if(0x0d == fs_buff_temp[fileindex] || 0x0a == fs_buff_temp[fileindex])break;
					fileindex ++;
				}
				continue;
			}
	    		fileindex += 6;
	    		while(1)
			{
	    			if(0x0d == fs_buff_temp[fileindex] || 0x0a == fs_buff_temp[fileindex])break;
	    			if(fs_buff_temp[fileindex] >= '0' && fs_buff_temp[fileindex] <= '9')
				{
	    				if(0 == type)Lsvrinfor.port = (Lsvrinfor.port << 4) + fs_buff_temp[fileindex] - '0';
	    				else Lsvrinfor.port1 = (Lsvrinfor.port1 << 4) + fs_buff_temp[fileindex] - '0';
	    			}
	    			else if(fs_buff_temp[fileindex] >= 'A' && fs_buff_temp[fileindex] <= 'F')
				{
	    				if(0 == type)Lsvrinfor.port = (Lsvrinfor.port << 4) + fs_buff_temp[fileindex] - 'A' + 0x0a;
	    				else Lsvrinfor.port1 = (Lsvrinfor.port1 << 4) + fs_buff_temp[fileindex] - 'A' + 0x0a;
	    			}
	    			else
				{
	    	  			user_debug("i:svr_file_init error2");
					return 3;
	      			}
	      			fileindex ++;
	      			if(fileindex >= filesize)
				{
					user_debug("i:svr_file_init error4");
					return 3;
				}
	    		}
	  	}
	  	else if('S' == fs_buff_temp[fileindex] && 'T' == fs_buff_temp[fileindex + 1] && 'T' == fs_buff_temp[fileindex + 2] && 'E' == fs_buff_temp[fileindex + 3] && 'L' == fs_buff_temp[fileindex + 4])
		  {//短信服务器号码 
		  	fileindex += 6;
		  	u8t1 = 0;
			while(1)
			{
				if(0x0d == fs_buff_temp[fileindex] || 0x0a == fs_buff_temp[fileindex])break;
				Lsvrinfor.svrtell_t[u8t1] = fs_buff_temp[fileindex];
				fileindex ++;
				u8t1 ++;
				if(u8t1 >= 12 || fileindex >= filesize)
				{
						user_debug("i:svr_file_init error5");
						return 4;
				}
			}
		  }
	  	else if('A' == fs_buff_temp[fileindex] && 'P' == fs_buff_temp[fileindex + 1] && 'N' == fs_buff_temp[fileindex + 2])
		  {//短信服务器号码 
		  		fileindex += 4;
		  		u8t1 = 0;
				while(1)
				{
					if(0x0d == fs_buff_temp[fileindex] || 0x0a == fs_buff_temp[fileindex])break;
					Lsvrinfor.apn[u8t1] = fs_buff_temp[fileindex];
					fileindex ++;
					u8t1 ++;
					if(u8t1 >= 16 || fileindex >= filesize)
					{
						user_debug("i:svr_file_init apn error");
						return 4;
					}
				}
		  }
	  	else if('T' == fs_buff_temp[fileindex] && 'T' == fs_buff_temp[fileindex + 1] && 'E' == fs_buff_temp[fileindex + 2] && 'L' == fs_buff_temp[fileindex + 3])
		  {//短信服务器号码 
		  	fileindex += 5;
		  	u8t1 = 0;
			while(1)
			{
				if(0x0d == fs_buff_temp[fileindex] || 0x0a == fs_buff_temp[fileindex])break;
				Lsvrinfor.ttell[u8t1] = fs_buff_temp[fileindex];
				fileindex ++;
				u8t1 ++;
				if(u8t1 >= 32 || fileindex >= filesize)
				{
					user_debug("i:svr_file_init ttell error");
					return 4;
				}
			}
		  }
	  	else if('S' == fs_buff_temp[fileindex] && 'R' == fs_buff_temp[fileindex + 1] && 'T' == fs_buff_temp[fileindex + 2] && 'E' == fs_buff_temp[fileindex + 3] && 'L' == fs_buff_temp[fileindex + 4])
		  {//短信服务器号码 
		  	fileindex += 6;
		  	u8t1 = 0;
			while(1)
			{
				if(0x0d == fs_buff_temp[fileindex] || 0x0a == fs_buff_temp[fileindex])break;
				Lsvrinfor.svrtell_r[u8t1] = fs_buff_temp[fileindex];
				fileindex ++;
				u8t1 ++;
				if(u8t1 >= 12 || fileindex >= filesize)
				{
					user_debug("i:svr_file_init error5");
					return 4;
				}
			}
		  }
	  	else if('T' == fs_buff_temp[fileindex] && 'E' == fs_buff_temp[fileindex + 1] && 'L' == fs_buff_temp[fileindex + 2] && '1' == fs_buff_temp[fileindex + 3])
		  {//用户器号码 
		  	fileindex += 5;
		  	u8t1 = 0;
			while(1)
			{
				if(0x0d == fs_buff_temp[fileindex] || 0x0a == fs_buff_temp[fileindex])break;
				Lsvrinfor.telx[0][u8t1] = fs_buff_temp[fileindex];
				fileindex ++;
				u8t1 ++;
				if(u8t1 >= 12 || fileindex >= filesize)
				{
					user_debug("i:svr_file_init error5");
					return 4;
				}
			}
		  }
	  	else if('M' == fs_buff_temp[fileindex] && 'M' == fs_buff_temp[fileindex + 1] && 'C' == fs_buff_temp[fileindex + 2])
		  {
		  	fileindex += 4;
		  	if(fs_buff_temp[fileindex] == '0')Lsvrinfor.media = 0;
		  	else if(fs_buff_temp[fileindex] == '1')Lsvrinfor.media = 1;
		  	u8t1 = 0;
			while(1)
			{
				if(0x0d == fs_buff_temp[fileindex] || 0x0a == fs_buff_temp[fileindex])break;
				fileindex ++;
			}
		  }
	  	else if('T' == fs_buff_temp[fileindex] && 'E' == fs_buff_temp[fileindex + 1] && 'L' == fs_buff_temp[fileindex + 2] && '2' == fs_buff_temp[fileindex + 3])
		  {//用户器号码 
		  	fileindex += 5;
		  	u8t1 = 0;
			while(1)
			{
				if(0x0d == fs_buff_temp[fileindex] || 0x0a == fs_buff_temp[fileindex])break;
				Lsvrinfor.telx[1][u8t1] = fs_buff_temp[fileindex];
				fileindex ++;
				u8t1 ++;
				if(u8t1 >= 12 || fileindex >= filesize)
				{
					user_debug("i:svr_file_init error5");
					return 4;
				}
			}
		  }
	  	else if('T' == fs_buff_temp[fileindex] && 'E' == fs_buff_temp[fileindex + 1] && 'L' == fs_buff_temp[fileindex + 2] && '3' == fs_buff_temp[fileindex + 3])
		  {//用户器号码 
		  	fileindex += 5;
		  	u8t1 = 0;
			while(1)
			{
				if(0x0d == fs_buff_temp[fileindex] || 0x0a == fs_buff_temp[fileindex])break;
				Lsvrinfor.telx[2][u8t1] = fs_buff_temp[fileindex];
				fileindex ++;
				u8t1 ++;
				if(u8t1 >= 12 || fileindex >= filesize)
				{
					user_debug("i:svr_file_init error5");
					return 4;
				}
			}
		  }
	  	else if('T' == fs_buff_temp[fileindex] && 'E' == fs_buff_temp[fileindex + 1] && 'L' == fs_buff_temp[fileindex + 2] && '4' == fs_buff_temp[fileindex + 3])
		  {//用户器号码 
		  	fileindex += 5;
		  	u8t1 = 0;
			while(1)
			{
				if(0x0d == fs_buff_temp[fileindex] || 0x0a == fs_buff_temp[fileindex])break;
				Lsvrinfor.telx[3][u8t1] = fs_buff_temp[fileindex];
				fileindex ++;
				u8t1 ++;
				if(u8t1 >= 12 || fileindex >= filesize)
				{
					user_debug("i:svr_file_init error5");
					return 4;
				}
			}
		  }
	  	else if('T' == fs_buff_temp[fileindex] && 'E' == fs_buff_temp[fileindex + 1] && 'L' == fs_buff_temp[fileindex + 2] && '5' == fs_buff_temp[fileindex + 3])
		  {//用户器号码 
		  	fileindex += 5;
		  	u8t1 = 0;
			while(1)
			{
				if(0x0d == fs_buff_temp[fileindex] || 0x0a == fs_buff_temp[fileindex])break;
				Lsvrinfor.telx[4][u8t1] = fs_buff_temp[fileindex];
				fileindex ++;
				u8t1 ++;
				if(u8t1 >= 12 || fileindex >= filesize)
				{
					user_debug("i:svr_file_init error5");
					return 4;
				}
			}
		  }
	  	else if('C' == fs_buff_temp[fileindex] && 'Y' == fs_buff_temp[fileindex + 1] && 'C' == fs_buff_temp[fileindex + 2] && 'L' == fs_buff_temp[fileindex + 3])
		  {//返回周期
		  	fileindex += 5;
		  	while(1)
			{
		    		if(0x0d == fs_buff_temp[fileindex] || 0x0a == fs_buff_temp[fileindex])break;
		    		if(fs_buff_temp[fileindex] >= '0' && fs_buff_temp[fileindex] <= '9')Lsvrinfor.backcircle = (Lsvrinfor.backcircle << 4) + fs_buff_temp[fileindex] - '0';
		    		else if(fs_buff_temp[fileindex] >= 'A' && fs_buff_temp[fileindex] <= 'F')Lsvrinfor.backcircle = (Lsvrinfor.backcircle << 4) + fs_buff_temp[fileindex] - 'A' + 0x0a;
		    		else
				{
		    	  		user_debug("i:svr_file_init error6");
					return 5;
		      		}
		      		fileindex ++;
		      		if(fileindex >= filesize)
				{
					user_debug("i:svr_file_init error7");
					return 6;
				}
		    	}
		  }
	  	fileindex ++;
	  	if(fileindex >= filesize)
		  {
		  	//user_debug("svr_file_init OK[%s,%d][%s,%d]",Lsvrinfor.svraddr, Lsvrinfor.port);
		  	if(strlen((s8 *)Lsvrinfor.svraddr1) < 8 || 0 == Lsvrinfor.port1)
			{
		  		strcpy((s8 *)Lsvrinfor.svraddr1, "39.108.229.40");
		      		Lsvrinfor.port1 = 9999;
		  		return 0xff;
		  	}
		  	return 0;
		  }
	}
}

u8 db_svr_save(void)
{
	s32 fpr,seekRet;
	u32 filesize,readLen;
	eat_fs_error_enum fserror;
	
		
	strcpy((s8 *)fs_buff_temp, "IP0:");
	strcat((s8 *)fs_buff_temp, Lsvrinfor.svraddr);
	strcat((s8 *)fs_buff_temp, "\r\n");
	strcat((s8 *)fs_buff_temp, "PORT0:");
	strcat((s8 *)fs_buff_temp, u32Str(Lsvrinfor.port));
	strcat((s8 *)fs_buff_temp, "\r\n");
	strcat((s8 *)fs_buff_temp, "IP1:");
	strcat((s8 *)fs_buff_temp, Lsvrinfor.svraddr1);
	strcat((s8 *)fs_buff_temp, "\r\n");
	strcat((s8 *)fs_buff_temp, "PORT1:");
	strcat((s8 *)fs_buff_temp, u32Str(Lsvrinfor.port1));
	strcat((s8 *)fs_buff_temp, "\r\n");
	strcat((s8 *)fs_buff_temp, "STTEL:");
	strcat((s8 *)fs_buff_temp, Lsvrinfor.svrtell_t);
	strcat((s8 *)fs_buff_temp, "\r\n");
	strcat((s8 *)fs_buff_temp, "SRTEL:");
	strcat((s8 *)fs_buff_temp, Lsvrinfor.svrtell_r);
	strcat((s8 *)fs_buff_temp, "\r\n");
	strcat((s8 *)fs_buff_temp, "TEL1:");
	strcat((s8 *)fs_buff_temp, Lsvrinfor.telx[0]);
	strcat((s8 *)fs_buff_temp, "\r\n");
	strcat((s8 *)fs_buff_temp, "TEL2:");
	strcat((s8 *)fs_buff_temp, Lsvrinfor.telx[1]);
	strcat((s8 *)fs_buff_temp, "\r\n");
	strcat((s8 *)fs_buff_temp, "TEL3:");
	strcat((s8 *)fs_buff_temp, Lsvrinfor.telx[2]);
	strcat((s8 *)fs_buff_temp, "\r\n");
	strcat((s8 *)fs_buff_temp, "TEL4:");
	strcat((s8 *)fs_buff_temp, Lsvrinfor.telx[3]);
	strcat((s8 *)fs_buff_temp, "\r\n");
	strcat((s8 *)fs_buff_temp, "TEL5:");
	strcat((s8 *)fs_buff_temp, Lsvrinfor.telx[4]);
	strcat((s8 *)fs_buff_temp, "\r\n");
	strcat((s8 *)fs_buff_temp, "CYCL:");
	strcat((s8 *)fs_buff_temp, u8Str(Lsvrinfor.backcircle));
	strcat((s8 *)fs_buff_temp, "\r\n");
	strcat((s8 *)fs_buff_temp, "APN:");
	strcat((s8 *)fs_buff_temp, (s8 *)Lsvrinfor.apn);
	strcat((s8 *)fs_buff_temp, "\r\n");
	strcat((s8 *)fs_buff_temp, "TTEL:");
	strcat((s8 *)fs_buff_temp, (s8 *)Lsvrinfor.ttell);
	strcat((s8 *)fs_buff_temp, "\r\n");
	strcat((s8 *)fs_buff_temp, "MMC:");
	if(0 == Lsvrinfor.media)strcat((s8 *)fs_buff_temp, "0");
	else strcat((s8 *)fs_buff_temp, "1");
	strcat((s8 *)fs_buff_temp, "\r\n");
	
	fpr =  eat_fs_Open(SVR_file, FS_READ_WRITE | FS_CREATE);
	if(fpr<EAT_FS_NO_ERROR){
		user_debug("i:db_update_save fileopen error");
		return 4;
	}
	eat_fs_Seek(fpr,0,EAT_FS_FILE_BEGIN);
	fserror = eat_fs_Write(fpr, fs_buff_temp, strlen(fs_buff_temp), &filesize);
	if(EAT_FS_NO_ERROR!= fserror|| strlen(fs_buff_temp) != filesize){
		user_debug("i:db_svr_default data write error");
		eat_fs_Close(fpr);
		return 5;
	}
	else{
	  fserror = eat_fs_Commit(fpr);
	  if(EAT_FS_NO_ERROR != fserror ){
	  	user_debug("i:db_svr_default commit error");
	  	eat_fs_Close(fpr);
		  return 6;
	  }
	  eat_fs_Close(fpr);
	  user_infor("e:db_svr_default OK");
		return 0;
	}
}
/*服务器采用默认信息
*
*****************************************/
u8 db_svr_default(void){
	
	
	Lsvrinfor.backcircle = 0;//默认为0
	strcpy((s8 *)Lsvrinfor.svraddr1, "39.108.229.40");
	Lsvrinfor.port1 = 9999;
	Lsvrinfor.media = 0;
	memset(Lsvrinfor.svrtell_t, 0, 12);
	memset(Lsvrinfor.svrtell_r, 0, 12);
	memset(Lsvrinfor.telx[0], 0, 12);
	memset(Lsvrinfor.telx[1], 0, 12);
	memset(Lsvrinfor.telx[2], 0, 12);
	memset(Lsvrinfor.telx[3], 0, 12);
	memset(Lsvrinfor.telx[4], 0, 12);
	memset(Lsvrinfor.apn, 0, 16);
	memset(Lsvrinfor.ttell, 0, 32);

	return db_svr_save();
}



void db_svr_addrset(u8 *addr){
	if(NULL == addr || strlen((s8 *)addr) >= 128)return;
	strcpy((s8 *)Lsvrinfor.svraddr, (s8 *)addr);
}
u8 *db_svr_addrget(void){
	if(0 == strlen((s8 *)Lsvrinfor.svraddr)){
		strcpy((s8 *)Lsvrinfor.svraddr, "106.3.226.182");
	}
	user_debug("i:AanIP=%s,%d", (s8 *)Lsvrinfor.svraddr, Lsvrinfor.port);
	return Lsvrinfor.svraddr;
}
void db_svr_addr1set(u8 *addr){
	if(NULL == addr || strlen((s8 *)addr) >= 128)return;
	strcpy((s8 *)Lsvrinfor.svraddr1, (s8 *)addr);
}
u8 *db_svr_addr1get(void){
	return Lsvrinfor.svraddr1;
}
void db_svr_sttelset(u8 *stel){
	if(NULL == stel || strlen((s8 *)stel) >= 128)return;
	strcpy((s8 *)Lsvrinfor.svrtell_t, (s8 *)stel);
}
u8 *db_svr_sttelget(void){
	return Lsvrinfor.svrtell_t;
}
void db_svr_srtelset(u8 *stel){
	if(NULL == stel || strlen((s8 *)stel) >= 128)return;
	strcpy((s8 *)Lsvrinfor.svrtell_r, (s8 *)stel);
}
u8 *db_svr_srtelget(void){
	return Lsvrinfor.svrtell_r;
}
void db_svr_portset(u32 port){
	Lsvrinfor.port = port;
}
u32 db_svr_portget(void){
	if(0 == Lsvrinfor.port)Lsvrinfor.port = 9999;
	if(Lsvrinfor.port != 30005){
	   strcpy((s8 *)Lsvrinfor.svraddr, "39.108.229.40");
	   Lsvrinfor.port = 30005;
	   db_svr_save();
  }
	return Lsvrinfor.port;
}
void db_svr_port1set(u32 port){
	Lsvrinfor.port1 = port;
}
u32 db_svr_port1get(void){
	return Lsvrinfor.port1;
}
void db_svr_cyclset(u32 cycl){
	Lsvrinfor.backcircle = cycl;
}
u32 db_svr_cyclget(void){
	return Lsvrinfor.backcircle;
}

void db_svr_telxset(u8 *tel, u8 telnum){
	if(0 == tel || strlen(tel) >= 24 || telnum >= 5)return;
	strcpy(Lsvrinfor.telx[telnum], (s8 *)tel);
}

void db_svr_apnset(u8 *apn){
	if(NULL == apn || strlen((s8 *)apn) >= 16)return;
	if('N' == *(apn) && 'U' == *(apn +1) && 'L' == *(apn + 2) && 'L' == *(apn + 3))Lsvrinfor.apn[0] = 0;
	else strcpy((s8 *)Lsvrinfor.apn, (s8 *)apn);
	user_debug("i:APN_set=%s", Lsvrinfor.apn);
}
u8 *db_svr_apnget(void){
	user_debug("i:APN_read=%s", Lsvrinfor.apn);
	return Lsvrinfor.apn;
}

void db_svr_mmcset(u8 flag){
	Lsvrinfor.media = flag;
}

u8 db_svr_mmcget(void){
	return Lsvrinfor.media;
}

void db_svr_ttellset(u8 *ttell){
	if(NULL == ttell  || strlen((s8 *)ttell) >= 32)return;
	strcpy((s8 *)Lsvrinfor.ttell, (s8 *)ttell);
}
u8 *db_svr_ttellget(void){
	if(NULL == Lsvrinfor.ttell || strlen(Lsvrinfor.ttell) < 3)return NULL;
	return Lsvrinfor.ttell;
}

/*GPS数据保存
*/
static _DBGPS dbgps;
#define gps_file L"C:\\gps" //记录行程信息
u8 db_gps_init(void)
{
	s32 fpr,seekRet;
	u32 filesize,readLen,dataindex;
	eat_fs_error_enum fserror;
	u8 data[8],datalen;
	
	dbgps.lac = 0;
	dbgps.lon = 0;
	dbgps.cellid = 0;
	dbgps.localid = 0;
	fpr =  eat_fs_Open(gps_file, FS_READ_ONLY);
	if(fpr<EAT_FS_NO_ERROR)
	{
		return 0;    
	}  
	
	fserror = (eat_fs_error_enum)eat_fs_GetFileSize(fpr,&filesize);
	if(filesize < 10)
	{
		eat_fs_Close(fpr);
		return 0;
	}
	if(EAT_FS_NO_ERROR != fserror)
	{
		user_debug("i:db_gps_init Return Error is %d",fserror);
		eat_fs_Close(fpr);
		return 1;
	}
	
	seekRet = eat_fs_Seek(fpr,0,EAT_FS_FILE_BEGIN);
	if(0>seekRet)
	{
		user_debug("i:db_gps_init:Seek File Pointer Fail");
		eat_fs_Close(fpr);
		return 2;
	}
	
	fserror = (eat_fs_error_enum)eat_fs_Read(fpr,fs_buff_temp,filesize,&readLen);
	if(EAT_FS_NO_ERROR != fserror ||filesize !=readLen)
	{	
		user_debug("i:db_gps_init:Read File Fail,and Return Error is %d,Readlen is %d",fserror,readLen);
		eat_fs_Close(fpr);
		return 3;
	}
	eat_fs_Close(fpr);
	
	dataindex = 0;
	while(1)
	{
		if('L' == fs_buff_temp[dataindex] && 'A' == fs_buff_temp[dataindex +1] && 'C' == fs_buff_temp[dataindex +2])
		{
			dataindex += 4;//跳过"LAC:"
			while(1)
			{
				if(0x0a == fs_buff_temp[dataindex] || 0x0d == fs_buff_temp[dataindex])break;
				if(fs_buff_temp[dataindex] >= '0' && fs_buff_temp[dataindex] <= '9')dbgps.lac = dbgps.lac * 10 + fs_buff_temp[dataindex] - '0';
				else return 4;
				dataindex ++;
				if(dataindex >= readLen)break;
			}
		}
		else if('L' == fs_buff_temp[dataindex] && 'O' == fs_buff_temp[dataindex +1] && 'N' == fs_buff_temp[dataindex +2])
		{
			dataindex += 4;//跳过"LON:"
			while(1)
			{
				if(0x0a == fs_buff_temp[dataindex] || 0x0d == fs_buff_temp[dataindex])break;
				if(fs_buff_temp[dataindex] >= '0' && fs_buff_temp[dataindex] <= '9')dbgps.lon = dbgps.lon * 10 + fs_buff_temp[dataindex] - '0';
				else return 5;
				dataindex ++;
				if(dataindex >= readLen)break;
			}
		}
		else if('C' == fs_buff_temp[dataindex] && 'I' == fs_buff_temp[dataindex +1] && 'D' == fs_buff_temp[dataindex +2]){
			dataindex += 4;//跳过"CID:"
			while(1){
				if(0x0a == fs_buff_temp[dataindex] || 0x0d == fs_buff_temp[dataindex])break;
				if(fs_buff_temp[dataindex] >= '0' && fs_buff_temp[dataindex] <= '9')dbgps.cellid = dbgps.cellid * 10 + fs_buff_temp[dataindex] - '0';
				else return 5;
				dataindex ++;
				if(dataindex >= readLen)break;
			}
		}
		else if('L' == fs_buff_temp[dataindex] && 'I' == fs_buff_temp[dataindex +1] && 'D' == fs_buff_temp[dataindex +2]){
			dataindex += 4;//跳过"LID:"
			while(1){
				if(0x0a == fs_buff_temp[dataindex] || 0x0d == fs_buff_temp[dataindex])break;
				if(fs_buff_temp[dataindex] >= '0' && fs_buff_temp[dataindex] <= '9')dbgps.localid = dbgps.localid * 10 + fs_buff_temp[dataindex] - '0';
				else return 5;
				dataindex ++;
				if(dataindex >= readLen)break;
			}
		}
		else dataindex ++;
		if(dataindex >= readLen)break;
	}
	//user_debug("GPS INIT[%d-%d]",dbgps.lac,dbgps.lon);
	return 0;
}

u8 db_gps_get(u32 *lac, u32 *loc){
	*lac = dbgps.lac;
	*loc = dbgps.lon;
	return 0;
}

u8 db_gpscell_get(u32 *lac, u32 *cel){
	*lac = dbgps.localid;
	*cel = dbgps.cellid;
	return 0;
}


u8 db_gps_save(void){
	s32 fpr,seekRet;
	u8 datatemp[24];
	u32 filesize,readLen;
	eat_fs_error_enum fserror;
	
  if(0 == dbgps.lac || 0 == dbgps.lon)return 1;
	strcpy((s8 *)fs_buff_temp, "LAC:");
	sprintf(datatemp,"%d", dbgps.lac);
	strcat((s8 *)fs_buff_temp, datatemp);
	strcat((s8 *)fs_buff_temp, "\r\n");
	strcat((s8 *)fs_buff_temp, "LON:");
	sprintf(datatemp,"%d", dbgps.lon);
	strcat((s8 *)fs_buff_temp, datatemp);
	strcat((s8 *)fs_buff_temp, "\r\n");
	strcat((s8 *)fs_buff_temp, "CID:");
	sprintf(datatemp,"%d", dbgps.cellid);
	strcat((s8 *)fs_buff_temp, datatemp);
	strcat((s8 *)fs_buff_temp, "\r\n");
	strcat((s8 *)fs_buff_temp, "LID:");
	sprintf(datatemp,"%d", dbgps.localid);
	strcat((s8 *)fs_buff_temp, datatemp);
	strcat((s8 *)fs_buff_temp, "\r\n");
	
	fpr =  eat_fs_Open(gps_file, FS_READ_WRITE | FS_CREATE);
	if(fpr<EAT_FS_NO_ERROR){
		user_debug("i:db_gps_save fileopen error");
		return 4;
	}
	eat_fs_Seek(fpr,0,EAT_FS_FILE_BEGIN);
	fserror = eat_fs_Write(fpr, fs_buff_temp, strlen(fs_buff_temp), &filesize);
	if(EAT_FS_NO_ERROR!= fserror|| strlen(fs_buff_temp) != filesize){
		user_debug("i:db_gps_save data write error");
		eat_fs_Close(fpr);
		return 5;
	}
	else{
	  fserror = eat_fs_Commit(fpr);
	  if(EAT_FS_NO_ERROR != fserror ){
	  	user_debug("i:db_gps_save commit error");
	  	eat_fs_Close(fpr);
		  return 6;
	  }
	  eat_fs_Close(fpr);
	  user_infor("e:db_gps_save OK");
		return 0;
	}
}

void db_gps_set(u32 lac, u32 loc){
	static u8 dbsave = 0;
	
	if(0 == lac || 0 == loc)return;
	if(Lenginespeedmax_get() < 600 && Lvehiclespeedmax_get() < 20)return;
	if(0 == dbsave){
		db_gps_save();
		dbsave = 0x55;
	}
}
void db_gps_cellsave(void){
	u32 cellid, localid;
	
	if(Lenginespeedmax_get() < 600 && Lvehiclespeedmax_get() < 20)return;
	cellid = 0;
	localid = 0;
	AT_CENG_CELL(&localid, &cellid);
	dbgps.cellid = cellid;
	dbgps.localid = localid;
}
/*行程数据
*行程上报时如果上报失败需要将数据保存到文件中
*下次系统启动时读出再次上报到服务器 确保该数据能正常到服务器
**********************************************************/
#define rout_file L"C:\\rout" //记录行程信息
u8 rout_fileclear(void){
	s32 fpr,seekRet;
	u32 filesize,readLen;
	eat_fs_error_enum fserror;
	u8 data[6];
  
       strcpy(data, "NULL");
	fpr =  eat_fs_Open(rout_file, FS_READ_WRITE | FS_CREATE);
	if(fpr<EAT_FS_NO_ERROR)
	{
		user_debug("i:rout_fileclear fileopen error");
		return 4;
	}
	eat_fs_Seek(fpr,0,EAT_FS_FILE_BEGIN);
	fserror = eat_fs_Write(fpr, data, strlen(data), &filesize);
	if(EAT_FS_NO_ERROR!= fserror)
	{
		user_debug("i:rout_fileclear data write error");
		eat_fs_Close(fpr);
		return 5;
	}
	else{
	  fserror = eat_fs_Commit(fpr);
	  if(EAT_FS_NO_ERROR != fserror ){
	  	user_debug("i:rout_fileclear commit error");
	  	eat_fs_Close(fpr);
		  return 6;
	  }
	  eat_fs_Close(fpr);
	  user_infor("e:rout_fileclear OK");
		return 0;
	}
	return 0;
}

u16 rout_fileread(u8 **dataptr)
{
	s32 fpr,seekRet;
	u32 filesize,readLen;
	eat_fs_error_enum fserror;
	u8 data[8],datalen;
	

	fpr =  eat_fs_Open(rout_file, FS_READ_ONLY);
	if(fpr<EAT_FS_NO_ERROR)
	{
		user_debug("i:rout_fileread no routfile");
		return 0;    
	}  
	
	fserror = (eat_fs_error_enum)eat_fs_GetFileSize(fpr,&filesize);
	if(filesize < 10)
	{
		eat_fs_Close(fpr);
		user_debug("i:rout_fileread no rout");
		return 0;
	}
	if(EAT_FS_NO_ERROR != fserror)
	{
		user_debug("i:rout_fileread Return Error is %d",fserror);
		eat_fs_Close(fpr);
		rout_fileclear();
		return 0;
	}
	
	seekRet = eat_fs_Seek(fpr,0,EAT_FS_FILE_BEGIN);
	if(0>seekRet)
	{
		user_debug("i:rout_fileread():Seek File Pointer Fail");
		eat_fs_Close(fpr);
		rout_fileclear();
		return 0;
	}
	
	fserror = (eat_fs_error_enum)eat_fs_Read(fpr,fs_buff_temp,filesize,&readLen);
	if(EAT_FS_NO_ERROR != fserror ||filesize !=readLen)
	{	
		user_debug("i:rout_fileread():Read File Fail,and Return Error is %d,Readlen is %d",fserror,readLen);
		eat_fs_Close(fpr);
		rout_fileclear();
		return 0;
	}
	eat_fs_Close(fpr);
	user_debug("lilei-Read rout file 05 01data\r\n");
	rout_fileclear();
	if(0xa5 == fs_buff_temp[0] && 0xa5 == fs_buff_temp[1]){
		*dataptr = fs_buff_temp;
		datalen = filesize & 0x00ff;
		user_infor("e:rout_fileread OK[%d]", datalen);
		return datalen;
	}
	user_debug("i:rout_file is NULL");
	return 0;
}

u8 rout_filesave(u8 *dataptr, u32 datalen)
{
	s32 fpr,seekRet;
	u32 filesize,readLen;
	eat_fs_error_enum fserror;
	u8 *savebuf;
	u16 savebuflen;

	if(NULL == dataptr || datalen < 1)return 1;
	savebuflen = rout_fileread(&savebuf);
	if(savebuflen + datalen > 1024)
	{
		user_debug("i:rout file full");
		return 1;
	}
	if(savebuflen)
	{
	    memcpy(savebuf + savebuflen, dataptr, datalen);
	    savebuflen += datalen;
  	}
  	else
	{
      		savebuf = dataptr;
      		savebuflen = datalen;
  	}
	fpr =  eat_fs_Open(rout_file, FS_READ_WRITE | FS_CREATE);
	if(fpr<EAT_FS_NO_ERROR)
	{
		user_debug("i:rout_filesave fileopen error");
		return 4;
	}
	eat_fs_Seek(fpr,0,EAT_FS_FILE_BEGIN);
	fserror = eat_fs_Write(fpr, savebuf, savebuflen, &filesize);
	if(EAT_FS_NO_ERROR!= fserror||savebuflen != filesize)
	{
		user_debug("i:rout_filesave data write error");
		eat_fs_Close(fpr);
		return 5;
	}
	else
	{
	  	fserror = eat_fs_Commit(fpr);
	  	if(EAT_FS_NO_ERROR != fserror ){
	  	user_debug("i:rout_filesave commit error");
	  	eat_fs_Close(fpr);
		return 6;
	  }
	  	eat_fs_Close(fpr);
	  	user_debug("i:rout_filesave OK[%d-%02x-%02x]",savebuflen, *savebuf, *(savebuf +1));
		return 0;
	}
	return 0;
}


/*
*保存读取发送数据二级缓存的内容
***************************************************************************/
static _DBFILE Ldbfile;
#define db_file1 L"C:\\dbfile1"
#define db_file2 L"C:\\dbfile2"
#define db_file3 L"C:\\dbfile3"
#define db_file4 L"C:\\dbfile4"
#define db_file5 L"C:\\dbfile5"
#define db_file6 L"C:\\dbfile6"
#define db_file7 L"C:\\dbfile7"
#define db_file8 L"C:\\dbfile8"
#define db_file9 L"C:\\dbfile9"
#define db_file0 L"C:\\dbfile0"
void db_fileinit(void){
	s32 fpr,seekRet;
	u32 filesize;
	eat_fs_error_enum fserror;
	
  Ldbfile.filenum = 0;
  for(Ldbfile.filenum = 0; Ldbfile.filenum < DB_FILE_MAX; Ldbfile.filenum ++)
  {
	    if(0 == Ldbfile.filenum)fpr =  eat_fs_Open(db_file0, FS_READ_ONLY);
	    else if(1 == Ldbfile.filenum)fpr =  eat_fs_Open(db_file1, FS_READ_ONLY);
	    else if(2 == Ldbfile.filenum)fpr =  eat_fs_Open(db_file2, FS_READ_ONLY);
	    else if(3 == Ldbfile.filenum)fpr =  eat_fs_Open(db_file3, FS_READ_ONLY);
	    else if(4 == Ldbfile.filenum)fpr =  eat_fs_Open(db_file4, FS_READ_ONLY);
	    else if(5 == Ldbfile.filenum)fpr =  eat_fs_Open(db_file5, FS_READ_ONLY);
	    else if(6 == Ldbfile.filenum)fpr =  eat_fs_Open(db_file6, FS_READ_ONLY);
	    else if(7 == Ldbfile.filenum)fpr =  eat_fs_Open(db_file7, FS_READ_ONLY);
	    else if(8 == Ldbfile.filenum)fpr =  eat_fs_Open(db_file8, FS_READ_ONLY);
	    else if(9 == Ldbfile.filenum)fpr =  eat_fs_Open(db_file9, FS_READ_ONLY);
	    else break;
	    if(fpr<EAT_FS_NO_ERROR)
	    {
	    	break;    
	    }  
	    
	    fserror = (eat_fs_error_enum)eat_fs_GetFileSize(fpr,&filesize);
	    eat_fs_Close(fpr);
	    if(filesize < 10)continue;
	    if(EAT_FS_NO_ERROR != fserror)
	    {
	    	user_debug("i:rout_fileread Return Error is %d",fserror);
	    	continue;
	    }
	    Ldbfile.filesize[Ldbfile.filenum] = filesize;
	}
}


u8 db_fileclear(u8 u8fileindex){
	s32 fpr,seekRet;
	u32 filesize,readLen;
	eat_fs_error_enum fserror;
	u8 data[6];
  
  if(0 == u8fileindex)fpr =  eat_fs_Open(db_file0, FS_READ_WRITE | FS_CREATE);
	else if(1 == u8fileindex)fpr =  eat_fs_Open(db_file1, FS_READ_WRITE | FS_CREATE);
	else if(2 == u8fileindex)fpr =  eat_fs_Open(db_file2, FS_READ_WRITE | FS_CREATE);
	else if(3 == u8fileindex)fpr =  eat_fs_Open(db_file3, FS_READ_WRITE | FS_CREATE);
	else if(4 == u8fileindex)fpr =  eat_fs_Open(db_file4, FS_READ_WRITE | FS_CREATE);
	else if(5 == u8fileindex)fpr =  eat_fs_Open(db_file5, FS_READ_WRITE | FS_CREATE);
	else if(6 == u8fileindex)fpr =  eat_fs_Open(db_file6, FS_READ_WRITE | FS_CREATE);
	else if(7 == u8fileindex)fpr =  eat_fs_Open(db_file7, FS_READ_WRITE | FS_CREATE);
	else if(8 == u8fileindex)fpr =  eat_fs_Open(db_file8, FS_READ_WRITE | FS_CREATE);
	else if(9 == u8fileindex)fpr =  eat_fs_Open(db_file9, FS_READ_WRITE | FS_CREATE);
	else return 1;
  strcpy(data, "NULL");
	if(fpr<EAT_FS_NO_ERROR){
		user_debug("i:db_fileclear fileopen error");
		return 4;
	}
	eat_fs_Seek(fpr,0,EAT_FS_FILE_BEGIN);
	fserror = eat_fs_Write(fpr, data, strlen(data), &filesize);
	if(EAT_FS_NO_ERROR!= fserror){
		user_debug("i:db_fileclear data write error");
		eat_fs_Close(fpr);
		return 5;
	}
	else{
	  fserror = eat_fs_Commit(fpr);
	  if(EAT_FS_NO_ERROR != fserror ){
	  	user_debug("i:db_fileclear commit error");
	  	eat_fs_Close(fpr);
		  return 6;
	  }
	  eat_fs_Close(fpr);
	  user_infor("e:db_fileclear OK");
	  Ldbfile.filesize[u8fileindex] = 0;
		return 0;
	}
	Ldbfile.filesize[u8fileindex] = 0;
	return 0;
}

/*
*数据保存
*注：data必须指向Lbigmemory
*************************************************************/
u8 db_filesave(u8 *data, u32 datalen, u8 flag){
	u8 u8fileindex;
	u32 dbfilesize;
	s32 fpr,seekRet;
	u32 filesize,filesize1,readLen;
	eat_fs_error_enum fserror;
	
	user_infor("e:db_filesave..");
	if(NULL == data || 0 == datalen || datalen >= DATA_LOG_FILE_MAX)return 1;
	if(1 == flag)
	{
	    for(u8fileindex = 0; u8fileindex < DB_FILE_MAX; u8fileindex ++)
	    {
	    		if(Ldbfile.filesize[u8fileindex] + datalen < DATA_LOG_FILE_MAX)break;
	    }
  	}
  	else
	{//必须单独一个文件存储
     		for(u8fileindex = 0; u8fileindex < DB_FILE_MAX; u8fileindex ++)
		{
	    		if(Ldbfile.filesize[u8fileindex] < 10)break;
	    	}
  	}
	if(u8fileindex >= DATA_LOG_FILE_MAX)
	{
		user_debug("i:db_filesave error[u8fileindex=%d]",u8fileindex);
		return 0x7f;
	}
	
	filesize1 = Ldbfile.filesize[u8fileindex];
	user_debug("lilei--save in %d files,datalen=%lu,old filesize1=%lu\r\n",u8fileindex,datalen,filesize1);
	if(filesize1 < 10)
	{//原文件中的数据无效
		dbfilesize = datalen;
	}
	else
	{//原文件中的数据有效
	  	for(dbfilesize = 0; dbfilesize < datalen; dbfilesize ++)
		{//2014-10-30 16:52 FangCuisong
	  		*(data + dbfilesize + filesize1) = *(data + dbfilesize);
	  	}
	  	fpr = -1;
		  if(0 == u8fileindex)fpr =  eat_fs_Open(db_file0, FS_READ_ONLY);
		  else if(1 == u8fileindex)fpr =  eat_fs_Open(db_file1, FS_READ_ONLY);
		  else if(2 == u8fileindex)fpr =  eat_fs_Open(db_file2, FS_READ_ONLY);
		  else if(3 == u8fileindex)fpr =  eat_fs_Open(db_file3, FS_READ_ONLY);
		  else if(4 == u8fileindex)fpr =  eat_fs_Open(db_file4, FS_READ_ONLY);
		  else if(5 == u8fileindex)fpr =  eat_fs_Open(db_file5, FS_READ_ONLY);
		  else if(6 == u8fileindex)fpr =  eat_fs_Open(db_file6, FS_READ_ONLY);
		  else if(7 == u8fileindex)fpr =  eat_fs_Open(db_file7, FS_READ_ONLY);
		  else if(8 == u8fileindex)fpr =  eat_fs_Open(db_file8, FS_READ_ONLY);
		  else if(9 == u8fileindex)fpr =  eat_fs_Open(db_file9, FS_READ_ONLY);
	  	  if(fpr<EAT_FS_NO_ERROR)
		  {
	  	  	user_debug("i:db_filesave openfile[%d] error", u8fileindex);
	    		return 1;    
	  	  }
	  	  fserror = (eat_fs_error_enum)eat_fs_GetFileSize(fpr,&filesize);
	  	  if(filesize != filesize1)
		  {
	  	 	user_debug("i:db_filesave filesize[%d-%d-%d] error", u8fileindex,filesize ,filesize1);
	  	 	eat_fs_Close(fpr);
	  	 	return 1;
	  	  }
	  	fserror = (eat_fs_error_enum)eat_fs_Read(fpr,data,filesize,&readLen);
	  	if(EAT_FS_NO_ERROR != fserror ||filesize !=readLen)
	  	{	
		   	user_debug("i:db_filesave():fileread[%d] error",u8fileindex);
		   	eat_fs_Close(fpr);
		   	return 1;
	  	}
	  	dbfilesize = filesize1 + datalen;
	  	eat_fs_Close(fpr);
  	}
  	if(dbfilesize >= DATA_LOG_FILE_MAX)return 3;
  	Ldbfile.filesize[u8fileindex] = dbfilesize;
  	fpr = -1;
  	if(0 == u8fileindex)fpr =  eat_fs_Open(db_file0, FS_READ_WRITE | FS_CREATE);
  	else if(1 == u8fileindex)fpr =  eat_fs_Open(db_file1, FS_READ_WRITE | FS_CREATE);
  	else if(2 == u8fileindex)fpr =  eat_fs_Open(db_file2, FS_READ_WRITE | FS_CREATE);
  	else if(3 == u8fileindex)fpr =  eat_fs_Open(db_file3, FS_READ_WRITE | FS_CREATE);
  	else if(4 == u8fileindex)fpr =  eat_fs_Open(db_file4, FS_READ_WRITE | FS_CREATE);
  	else if(5 == u8fileindex)fpr =  eat_fs_Open(db_file5, FS_READ_WRITE | FS_CREATE);
  	else if(6 == u8fileindex)fpr =  eat_fs_Open(db_file6, FS_READ_WRITE | FS_CREATE);
  	else if(7 == u8fileindex)fpr =  eat_fs_Open(db_file7, FS_READ_WRITE | FS_CREATE);
  	else if(8 == u8fileindex)fpr =  eat_fs_Open(db_file8, FS_READ_WRITE | FS_CREATE);
  	else if(9 == u8fileindex)fpr =  eat_fs_Open(db_file9, FS_READ_WRITE | FS_CREATE);
  	if(fpr<EAT_FS_NO_ERROR)
  	{
		user_debug("i:db_filesave fileopen error");
		return 4;
  	}
  	eat_fs_Seek(fpr,0,EAT_FS_FILE_BEGIN);
  	fserror = eat_fs_Write(fpr, data, dbfilesize, &filesize);
  	if(EAT_FS_NO_ERROR!= fserror||dbfilesize != filesize)
  	{
		user_debug("i:db_filesave data write error");
		eat_fs_Close(fpr);
		return 5;
  	}
  	else
  	{
	  	fserror = eat_fs_Commit(fpr);
	  	if(EAT_FS_NO_ERROR != fserror )
	  	{
	  		user_debug("i:db_filesave commit error");
	  		eat_fs_Close(fpr);
			return 6;
	  	}
	  	eat_fs_Close(fpr);
	  	user_debug("i:db_filesave[%d,%d,%d]", datalen,dbfilesize,u8fileindex);
		return 0;
  	}
}

/*
*读取二级缓存中的数据
********************************************************/
u32 db_fileread(u8 *data)
{
	u8 u8fileindex;
	s32 fpr,seekRet;
	u32 filesize,readLen;
	eat_fs_error_enum fserror;
	
	if(NULL == data)return;
	for(u8fileindex = 0; u8fileindex < DB_FILE_MAX; u8fileindex ++)
	{
		if(Ldbfile.filesize[u8fileindex] > 10)break;
	}
	if(u8fileindex >= DB_FILE_MAX)
	{
		user_debug("lilei-二级缓存无数据\r\n");
		return 0;//二级缓存无数据
	}
	//大数据读取
	fpr = -1;
	if(0 == u8fileindex)fpr =  eat_fs_Open(db_file0, FS_READ_ONLY);
	else if(1 == u8fileindex)fpr =  eat_fs_Open(db_file1, FS_READ_ONLY);
	else if(2 == u8fileindex)fpr =  eat_fs_Open(db_file2, FS_READ_ONLY);
	else if(3 == u8fileindex)fpr =  eat_fs_Open(db_file3, FS_READ_ONLY);
	else if(4 == u8fileindex)fpr =  eat_fs_Open(db_file4, FS_READ_ONLY);
	else if(5 == u8fileindex)fpr =  eat_fs_Open(db_file5, FS_READ_ONLY);
	else if(6 == u8fileindex)fpr =  eat_fs_Open(db_file6, FS_READ_ONLY);
	else if(7 == u8fileindex)fpr =  eat_fs_Open(db_file7, FS_READ_ONLY);
	else if(8 == u8fileindex)fpr =  eat_fs_Open(db_file8, FS_READ_ONLY);
	else if(9 == u8fileindex)fpr =  eat_fs_Open(db_file9, FS_READ_ONLY);
	if(fpr<EAT_FS_NO_ERROR)
	{
	   	user_debug("i:db_filesave openfile[%d] error", u8fileindex);
	  	return 0;    
	}
	fserror = (eat_fs_error_enum)eat_fs_GetFileSize(fpr,&filesize);
	if(filesize != Ldbfile.filesize[u8fileindex])
	{
	  	 user_infor("e:db_fileread filesize[%d-%d-%d] error", u8fileindex,filesize ,Ldbfile.filesize[u8fileindex]);
	  	 eat_fs_Close(fpr);
	  	 db_fileclear(u8fileindex);
	  	 return 0;
	 }
	 fserror = (eat_fs_error_enum)eat_fs_Read(fpr,data,filesize,&readLen);
	 if(EAT_FS_NO_ERROR != fserror ||filesize !=readLen)
	 {	
 	     user_debug("i:db_fileread():fileread[%d] error",u8fileindex);
	     eat_fs_Close(fpr);
	     db_fileclear(u8fileindex);
		   return 0;
	 }
	 eat_fs_Close(fpr);
	  user_debug("i:db_fileread OK[%d,%d]",u8fileindex, Ldbfile.filesize[u8fileindex]);
	 db_fileclear(u8fileindex);
	 return filesize;
}

/*检测二级缓存是否有数据
****************************************************/
u8 db_filecheck(void){
	u8 u8fileindex;
	
	for(u8fileindex = 0; u8fileindex < DB_FILE_MAX; u8fileindex ++){
		if(Ldbfile.filesize[u8fileindex] > 10)return 1;
	}
	return 0;
}




#define db_testfile L"C:\\test"
#define db_testfile1 L"C:\\test1"
#define db_testfile2 L"C:\\test2.txt"
void db_test(void){
	s32 fpr;
	
	fpr =  eat_fs_Open(db_testfile, FS_CREATE | FS_READ_WRITE);
	if(fpr < EAT_FS_NO_ERROR){
		
		user_debug("i:db_test fail[%d]",fpr);
	}
	else{
		eat_fs_Close(fpr);
		user_debug("i:db_test OK");
	}
	
	fpr =  eat_fs_Open(db_testfile1, FS_CREATE | FS_READ_WRITE);
	if(fpr < EAT_FS_NO_ERROR){
		
		user_debug("i:db_test1 fail[%d]",fpr);
	}
	else{
		eat_fs_Close(fpr);
		user_debug("i:db_test1 OK");
	}
	
	fpr =  eat_fs_Open(db_testfile2, FS_CREATE | FS_READ_WRITE);
	if(fpr < EAT_FS_NO_ERROR){
		
		user_debug("i:db_test1 fai2[%d]",fpr);
	}
	else{
		eat_fs_Close(fpr);
		user_debug("i:db_test2 OK");
	}
	
	fpr =  eat_fs_Open(update_file, FS_CREATE | FS_READ_WRITE);
	if(fpr < EAT_FS_NO_ERROR){
		 user_debug("i:db_update[%d] fileopen error",fpr);
		 return;
	}
	else{
		eat_fs_Close(fpr);
		user_debug("i:db_update_ok");
	}
}




/*
OBD数据处理
*********************************************************/
static _OBD_DB Lobddb;
static _OBD_DB Lobddb_T;//临时数据
#define OBD_file L"C:\\obd_db.txt"

u32 db_obd_file_read(void){
	s32 fpr,seekRet;
	u32 filesize,readLen;
	eat_fs_error_enum fserror;
	

	fpr =  eat_fs_Open(OBD_file, FS_READ_ONLY);
	if(fpr < EAT_FS_NO_ERROR){
		fpr =  eat_fs_Open(OBD_file,FS_CREATE);
		if(fpr<EAT_FS_NO_ERROR){
			user_debug("i:Open OBD_File Fail,and Return Error is %d",fpr);
			return 0;
		}
		eat_fs_Close(fpr);
		return 0;    
	}  
	
	fserror = (eat_fs_error_enum)eat_fs_GetFileSize(fpr,&filesize);
	if(0 == filesize || filesize >= 1024){
		eat_fs_Close(fpr);
		return 2;
	}
	if(EAT_FS_NO_ERROR != fserror)
	{
		user_debug("i:Get SVR_File Size Fail,and Return Error is %d",fserror);
		eat_fs_Close(fpr);
		return 0;
	}
	
	seekRet = eat_fs_Seek(fpr,0,EAT_FS_FILE_BEGIN);
	if(0>seekRet)
	{
		user_debug("i:eat_fs_Seek():Seek File Pointer Fail");
		eat_fs_Close(fpr);
		return 0;
	}
	
	fserror = (eat_fs_error_enum)eat_fs_Read(fpr,fs_buff_temp,filesize,&readLen);
	if(EAT_FS_NO_ERROR != fserror ||filesize !=readLen)
	{	
		user_debug("i:eat_fs_Read():Read File Fail,and Return Error is %d,Readlen is %d",fserror,readLen);
		eat_fs_Close(fpr);
		return 0;
	}
	eat_fs_Close(fpr);
	return readLen;
}
u8 db_obd_init(void){
	u32 filesize,fileindex;
	u8 u8t1;
	u8 type;
	
	filesize = db_obd_file_read();
	if(filesize < 12){
		user_debug("i:db_obd_init to default");
		return 1;
	}
	fileindex = 0;
	Lobddb.engine_on_time_135 = 0;//总运行时间
	Lobddb.engine_on_time_136 = 0;//短期运行时间
	Lobddb.fuel_used_138 = 0;//总油耗
	Lobddb.fuel_used_139 = 0;//短期油耗
	Lobddb.fuel_used_av_141 = 0;//总平均油耗
	Lobddb.fuel_used_av_142 = 0;//短期平均油耗
	Lobddb.dist_147 = 0;//总里程
	Lobddb.dist_148 = 0;//短期里程
	//GPS数据只是临时使用 不需要做任何保存
	Lobddb.gpstime = 0;
	Lobddb.gpslon = 0;
	Lobddb.gpslat = 0;
	
	Lobddb_T.engine_on_time_135 = 0;//总运行时间
	Lobddb_T.engine_on_time_136 = 0;//短期运行时间
	Lobddb_T.fuel_used_138 = 0;//总油耗
	Lobddb_T.fuel_used_139 = 0;//短期油耗
	Lobddb_T.fuel_used_av_141 = 0;//总平均油耗
	Lobddb_T.fuel_used_av_142 = 0;//短期平均油耗
	Lobddb_T.dist_147 = 0;//总里程
	Lobddb_T.dist_148 = 0;//短期里程
	//GPS数据只是临时使用 不需要做任何保存
	Lobddb_T.gpstime = 0;
	Lobddb_T.gpslon = 0;
	Lobddb_T.gpslat = 0;
	
	user_debug("i:db_obd_init");
	while(1)
	{
		if('1' == fs_buff_temp[fileindex] && '3' == fs_buff_temp[fileindex + 1] && '5' == fs_buff_temp[fileindex + 2])
		{//总运行时间
	    		fileindex += 4;
	    		while(1)
			{
	    			if(0x0d == fs_buff_temp[fileindex] || 0x0a == fs_buff_temp[fileindex])break;
	    			if(fs_buff_temp[fileindex] >= '0' && fs_buff_temp[fileindex] <= '9')
				{
	    				Lobddb.engine_on_time_135 = Lobddb.engine_on_time_135 * 10 + (fs_buff_temp[fileindex] - '0');
	      			}
	    			else
				{
	    	  			user_debug("i:obd_file_init error3");
					return 3;
	      			}
	      			fileindex ++;
	      			if(fileindex >= filesize)
				{
					user_debug("i:obd_file_init error4");
					return 4;
				}
	    		}
	    		Lobddb_T.engine_on_time_135 = Lobddb.engine_on_time_135;
	  	}
	  	else if('1' == fs_buff_temp[fileindex] && '3' == fs_buff_temp[fileindex + 1] && '6' == fs_buff_temp[fileindex + 2])
		{//短期运行时间
	    		fileindex += 4;
	    		while(1)
			{
	    			if(0x0d == fs_buff_temp[fileindex] || 0x0a == fs_buff_temp[fileindex])break;
	    			if(fs_buff_temp[fileindex] >= '0' && fs_buff_temp[fileindex] <= '9')
				{
	    				Lobddb.engine_on_time_136 = Lobddb.engine_on_time_136 * 10 + (fs_buff_temp[fileindex] - '0');
	      			}
	    			else
				{
	    	  			user_debug("i:obd_file_init error4");
					return 5;
	      			}
	      			fileindex ++;
	      			if(fileindex >= filesize)
				{
					user_debug("i:obd_file_init error4");
					return 6;
				}
	    		}
	    		Lobddb_T.engine_on_time_136 = Lobddb.engine_on_time_136;
	  	}
	  	else if('1' == fs_buff_temp[fileindex] && '3' == fs_buff_temp[fileindex + 1] && '8' == fs_buff_temp[fileindex + 2])
		{//总油耗
	    		fileindex += 4;
	    		while(1)
			{
	    			if(0x0d == fs_buff_temp[fileindex] || 0x0a == fs_buff_temp[fileindex])break;
	    			if(fs_buff_temp[fileindex] >= '0' && fs_buff_temp[fileindex] <= '9')
				{
	    				Lobddb.fuel_used_138 = Lobddb.fuel_used_138 * 10 + (fs_buff_temp[fileindex] - '0');
	      			}
	    			else
				{
	    	  			user_debug("i:obd_file_init error7");
					return 7;
	      			}
	      			fileindex ++;
	      			if(fileindex >= filesize)
				{
					user_debug("i:obd_file_init error4");
					return 8;
				}
	    		}
	    		Lobddb_T.fuel_used_138 = Lobddb.fuel_used_138;
	  	}
	  	else if('1' == fs_buff_temp[fileindex] && '3' == fs_buff_temp[fileindex + 1] && '9' == fs_buff_temp[fileindex + 2])
		{//短期油耗
	    		fileindex += 4;
	    		while(1)
			{
	    			if(0x0d == fs_buff_temp[fileindex] || 0x0a == fs_buff_temp[fileindex])break;
	    			if(fs_buff_temp[fileindex] >= '0' && fs_buff_temp[fileindex] <= '9')
				{
	    				Lobddb.fuel_used_139 = Lobddb.fuel_used_139 * 10 + (fs_buff_temp[fileindex] - '0');
	      			}
	    			else
				{
	    	  			user_debug("i:obd_file_init error9");
					return 9;
	      			}
	      			fileindex ++;
	      			if(fileindex >= filesize)
				{
					user_debug("i:obd_file_init error10");
					return 10;
				}
	    		}
	    		Lobddb_T.fuel_used_139 = Lobddb.fuel_used_139;
	  	}
	  	else if('1' == fs_buff_temp[fileindex] && '4' == fs_buff_temp[fileindex + 1] && '1' == fs_buff_temp[fileindex + 2])
		{//总的平均油耗
	    		fileindex += 4;
	    		while(1)
			{
	    			if(0x0d == fs_buff_temp[fileindex] || 0x0a == fs_buff_temp[fileindex])break;
	    			if(fs_buff_temp[fileindex] >= '0' && fs_buff_temp[fileindex] <= '9')
				{
	    				Lobddb.fuel_used_av_141 = Lobddb.fuel_used_av_141 * 10 + (fs_buff_temp[fileindex] - '0');
	      			}
	    			else
				{
	    	  			user_debug("i:obd_file_init error11");
					return 11;
	      			}
	      			fileindex ++;
	      			if(fileindex >= filesize)
				{
					user_debug("i:obd_file_init error12");
					return 12;
				}
	    		}
	    		Lobddb_T.fuel_used_av_141 = Lobddb.fuel_used_av_141;
	  	}
	  	else if('1' == fs_buff_temp[fileindex] && '4' == fs_buff_temp[fileindex + 1] && '2' == fs_buff_temp[fileindex + 2])
		{//短期平均油耗
	    		fileindex += 4;
	    		while(1)
			{
	    			if(0x0d == fs_buff_temp[fileindex] || 0x0a == fs_buff_temp[fileindex])break;
	    			if(fs_buff_temp[fileindex] >= '0' && fs_buff_temp[fileindex] <= '9')
				{
	    				Lobddb.fuel_used_av_142 = Lobddb.fuel_used_av_142 * 10 + (fs_buff_temp[fileindex] - '0');
	      			}
	    			else
				{
	    	  			user_debug("i:obd_file_init errorr13");
					return 13;
	      			}
	      			fileindex ++;
	      			if(fileindex >= filesize)
				{
					user_debug("i:obd_file_init error4");
					return 14;
				}
	    		}
	    		Lobddb_T.fuel_used_av_142 = Lobddb.fuel_used_av_142;
	  	}//
	  	else if('1' == fs_buff_temp[fileindex] && '4' == fs_buff_temp[fileindex + 1] && '7' == fs_buff_temp[fileindex + 2])
		{//总里程
	    		fileindex += 4;
	    		while(1)
			{
	    			if(0x0d == fs_buff_temp[fileindex] || 0x0a == fs_buff_temp[fileindex])break;
	    			if(fs_buff_temp[fileindex] >= '0' && fs_buff_temp[fileindex] <= '9')
				{
	    				Lobddb.dist_147 = Lobddb.dist_147 * 10 + (fs_buff_temp[fileindex] - '0');
	      			}
	    			else
				{
	    	  			user_debug("i:obd_file_init error15");
					return 15;
	      			}
	      			fileindex ++;
	      			if(fileindex >= filesize)
				{
					user_debug("i:obd_file_init error4");
					return 16;
				}
	    		}
	    		Lobddb_T.dist_147 = Lobddb.dist_147;
	  	}
	  else if('1' == fs_buff_temp[fileindex] && '4' == fs_buff_temp[fileindex + 1] && '8' == fs_buff_temp[fileindex + 2])
	  {//短期里程
	    	fileindex += 4;
	    	while(1)
		{
	    		if(0x0d == fs_buff_temp[fileindex] || 0x0a == fs_buff_temp[fileindex])break;
	    		if(fs_buff_temp[fileindex] >= '0' && fs_buff_temp[fileindex] <= '9')
			{
	    			Lobddb.dist_148 = Lobddb.dist_148 * 10 + (fs_buff_temp[fileindex] - '0');
	      		}
	    		else
			{
	    	  		user_debug("i:obd_file_init error17");
				return 17;
	      		}
	      		fileindex ++;
	      		if(fileindex >= filesize)
			{
					user_debug("i:obd_file_init error18");
					return 18;
			}
	    	}
	    	Lobddb_T.dist_148 = Lobddb.dist_148;
	  }
	  fileindex ++;
	  if(fileindex >= filesize)
	  {
	  	return 0;
	  }
	}
}

/*清除GPS起始位置数据
*2015/7/2 7:06 Fangcuisong
*/
void db_obd_gpsclr(void){
	Lobddb.gpstime = 0;
	Lobddb.gpslon = 0;
	Lobddb.gpslat = 0;
	Lobddb_T.gpstime = 0;
	Lobddb_T.gpslon = 0;
	Lobddb_T.gpslat = 0;
}

void db_obd_gpsset(u32 gpstime, u32 gpslat, u32 gpslon){
	if(Lobddb.gpstime != 0){
		Lobddb.gpstime = gpstime;
		Lobddb.gpslat = gpslat;
		Lobddb.gpslon = gpslon;
	}
}


u8 db_obd_insert(u8 id, u32 data){
	u8 sendlen, senddata[80];
	
	if(135 == id)Lobddb_T.engine_on_time_135 = data;
	else if(136 == id)Lobddb_T.engine_on_time_136 = data;
	else if(138 == id)Lobddb_T.fuel_used_138 = data;
	else if(139 == id)Lobddb_T.fuel_used_139 = data;
	else if(141 == id)Lobddb_T.fuel_used_av_141 = data;
	else if(142 == id)Lobddb_T.fuel_used_av_142 = data;
	else if(147 == id){
		Lobddb_T.dist_147 = data;
		if((Lobddb_T.dist_147 + 5) < Lobddb.dist_147){//数据异常
			sendlen = 0;
			senddata[sendlen ++] = 0x2c;
			senddata[sendlen ++] = 0x07;
			senddata[sendlen ++] = 135;
			senddata[sendlen ++] = (Lobddb.engine_on_time_135 >> 24) & 0x00ff;
			senddata[sendlen ++] = (Lobddb.engine_on_time_135 >> 16) & 0x00ff;
			senddata[sendlen ++] = (Lobddb.engine_on_time_135 >> 8) & 0x00ff;
			senddata[sendlen ++] = (Lobddb.engine_on_time_135 >> 0) & 0x00ff;
			senddata[sendlen ++] = 136;
			senddata[sendlen ++] = (Lobddb.engine_on_time_136 >> 24) & 0x00ff;
			senddata[sendlen ++] = (Lobddb.engine_on_time_136 >> 16) & 0x00ff;
			senddata[sendlen ++] = (Lobddb.engine_on_time_136 >> 8) & 0x00ff;
			senddata[sendlen ++] = (Lobddb.engine_on_time_136 >> 0) & 0x00ff;
			senddata[sendlen ++] = 138;
			senddata[sendlen ++] = (Lobddb.fuel_used_138 >> 24) & 0x00ff;
			senddata[sendlen ++] = (Lobddb.fuel_used_138 >> 16) & 0x00ff;
			senddata[sendlen ++] = (Lobddb.fuel_used_138 >> 8) & 0x00ff;
			senddata[sendlen ++] = (Lobddb.fuel_used_138 >> 0) & 0x00ff;
			senddata[sendlen ++] = 139;
			senddata[sendlen ++] = (Lobddb.fuel_used_139 >> 24) & 0x00ff;
			senddata[sendlen ++] = (Lobddb.fuel_used_139 >> 16) & 0x00ff;
			senddata[sendlen ++] = (Lobddb.fuel_used_139 >> 8) & 0x00ff;
			senddata[sendlen ++] = (Lobddb.fuel_used_139 >> 0) & 0x00ff;
			senddata[sendlen ++] = 141;
			senddata[sendlen ++] = (Lobddb.fuel_used_av_141 >> 24) & 0x00ff;
			senddata[sendlen ++] = (Lobddb.fuel_used_av_141 >> 16) & 0x00ff;
			senddata[sendlen ++] = (Lobddb.fuel_used_av_141 >> 8) & 0x00ff;
			senddata[sendlen ++] = (Lobddb.fuel_used_av_141 >> 0) & 0x00ff;
			senddata[sendlen ++] = 142;
			senddata[sendlen ++] = (Lobddb.fuel_used_av_142 >> 24) & 0x00ff;
			senddata[sendlen ++] = (Lobddb.fuel_used_av_142 >> 16) & 0x00ff;
			senddata[sendlen ++] = (Lobddb.fuel_used_av_142 >> 8) & 0x00ff;
			senddata[sendlen ++] = (Lobddb.fuel_used_av_142 >> 0) & 0x00ff;
			senddata[sendlen ++] = 147;
			senddata[sendlen ++] = (Lobddb.dist_147 >> 24) & 0x00ff;
			senddata[sendlen ++] = (Lobddb.dist_147 >> 16) & 0x00ff;
			senddata[sendlen ++] = (Lobddb.dist_147 >> 8) & 0x00ff;
			senddata[sendlen ++] = (Lobddb.dist_147 >> 0) & 0x00ff;
			senddata[sendlen ++] = 148;
			senddata[sendlen ++] = (Lobddb.dist_148 >> 24) & 0x00ff;
			senddata[sendlen ++] = (Lobddb.dist_148 >> 16) & 0x00ff;
			senddata[sendlen ++] = (Lobddb.dist_148 >> 8) & 0x00ff;
			senddata[sendlen ++] = (Lobddb.dist_148 >> 0) & 0x00ff;
			senddata[sendlen ++] = 162;
			senddata[sendlen ++] = (Lobddb.gpstime >> 24) & 0x00ff;
			senddata[sendlen ++] = (Lobddb.gpstime >> 16) & 0x00ff;
			senddata[sendlen ++] = (Lobddb.gpstime >> 8) & 0x00ff;
			senddata[sendlen ++] = (Lobddb.gpstime >> 0) & 0x00ff;
			senddata[sendlen ++] = 157;
			senddata[sendlen ++] = (Lobddb.gpslon >> 24) & 0x00ff;
			senddata[sendlen ++] = (Lobddb.gpslon >> 16) & 0x00ff;
			senddata[sendlen ++] = (Lobddb.gpslon >> 8) & 0x00ff;
			senddata[sendlen ++] = (Lobddb.gpslon >> 0) & 0x00ff;
			senddata[sendlen ++] = 159;
			senddata[sendlen ++] = (Lobddb.gpslat >> 24) & 0x00ff;
			senddata[sendlen ++] = (Lobddb.gpslat >> 16) & 0x00ff;
			senddata[sendlen ++] = (Lobddb.gpslat >> 8) & 0x00ff;
			senddata[sendlen ++] = (Lobddb.gpslat >> 0) & 0x00ff;
			obd_write(senddata, sendlen);
	  }
		else{//数据正常
			Lobddb.engine_on_time_135 = Lobddb_T.engine_on_time_135;
			Lobddb.engine_on_time_136 = Lobddb_T.engine_on_time_136;
			Lobddb.fuel_used_138 = Lobddb_T.fuel_used_138;
			Lobddb.fuel_used_139 = Lobddb_T.fuel_used_139;
			Lobddb.fuel_used_av_141 = Lobddb_T.fuel_used_av_141;
			Lobddb.fuel_used_av_142 = Lobddb_T.fuel_used_av_142;
			Lobddb.dist_147 = Lobddb_T.dist_147;
			Lobddb.dist_148 = Lobddb_T.dist_148;
		}
	}
	else if(148 == id)Lobddb_T.dist_148 = data;
	return 0;
}

u8 db_obd_reset(void){
	user_debug("i:db_obd_reset >>");
	Lobddb.engine_on_time_135 = 0;//总运行时间
	Lobddb.engine_on_time_136 = 0;//短期运行时间
	Lobddb.fuel_used_138 = 0;//总油耗
	Lobddb.fuel_used_139 = 0;//短期油耗
	Lobddb.fuel_used_av_141 = 0;//总平均油耗
	Lobddb.fuel_used_av_142 = 0;//短期平均油耗
	Lobddb.dist_147 = 0;//总里程
	Lobddb.dist_148 = 0;//短期里程
	
	db_obd_save();
	return 0;
}

u8 db_obd_save(void)
{
	s32 fpr;
	eat_fs_error_enum fserror;
	u32 filesize;
	char datatem[64];
	
	
	
	sprintf(datatem, "135:%d\r\n",Lobddb.engine_on_time_135);
	strcpy((s8 *)fs_buff_temp, datatem);
	sprintf(datatem, "136:%d\r\n",Lobddb.engine_on_time_136);
	strcat((s8 *)fs_buff_temp, datatem);
	sprintf(datatem, "138:%d\r\n",Lobddb.fuel_used_138);
	strcat((s8 *)fs_buff_temp, datatem);
	sprintf(datatem, "139:%d\r\n",Lobddb.fuel_used_139);
	strcat((s8 *)fs_buff_temp, datatem);
	sprintf(datatem, "141:%d\r\n",Lobddb.fuel_used_av_141);
	strcat((s8 *)fs_buff_temp, datatem);
	sprintf(datatem, "142:%d\r\n",Lobddb.fuel_used_av_142);
	strcat((s8 *)fs_buff_temp, datatem);
	sprintf(datatem, "147:%d\r\n",Lobddb.dist_147);
	strcat((s8 *)fs_buff_temp, datatem);
	sprintf(datatem, "148:%d\r\n",Lobddb.dist_148);
	strcat((s8 *)fs_buff_temp, datatem);
	
	user_debug("i:db_obd_save dist=%d", Lobddb.dist_147);
	fpr =  eat_fs_Open(OBD_file, FS_READ_WRITE | FS_CREATE);
	if(fpr < EAT_FS_NO_ERROR)
	{
		user_debug("i:db_obd_save fileopen error");
		return 4;
	}
	eat_fs_Seek(fpr,0,EAT_FS_FILE_BEGIN);
	
	fserror = eat_fs_Write(fpr, fs_buff_temp, strlen(fs_buff_temp), &filesize);
	if(EAT_FS_NO_ERROR!= fserror|| strlen(fs_buff_temp) != filesize)
	{
		user_debug("i:db_obd_save data write error");
		eat_fs_Close(fpr);
		return 5;
	}
	else
	{
	  	fserror = eat_fs_Commit(fpr);
	  	if(EAT_FS_NO_ERROR != fserror ){
	  	user_debug("i:db_obd_save commit error");
	  	eat_fs_Close(fpr);
		  return 6;
	  }
	  eat_fs_Close(fpr);
	  user_infor("e:db_obd_save OK");
		return 0;
	}
}


