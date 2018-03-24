/* car-eye车辆管理平台 
 * car-eye车辆管理公共平台   www.car-eye.cn
 * car-eye开源网址:  https://github.com/Car-eye-admin
 * Copyright car-eye 车辆管理平台  2018 
 */

#ifndef _APP_UPDATE_
#define _APP_UPDATE_

#ifdef __cplusplus
extern "C"{
#endif
typedef struct{
	u8 flag;
	u8 *data;
	u8 name[35];
	u32 dataindex;//数据字节个数
	u32 frameindex;//帧序号
	u32 datalen;//数据总长度
	u32 cs;//数据总校验
}_APPUPDATE;
 

void update_init(void);
u8 update_start(u32 datalen, u8 type, u8 *name);
void update_end(void);
u8 update_datain(u8 *data, u16 datalen);
u8 update_datainEx(u8 *data, u16 datalen, u32 findex);
u8 update_cs(u32 cs);
u8 update_do(void);

u8 update_obdlicense(void);
u8 update_obdapp(void);
u8 update_obdmusic(void);
#ifdef __cplusplus
}
#endif
#endif


