/* car-eye车辆管理平台 
 * car-eye车辆管理公共平台   www.car-eye.cn
 * car-eye开源网址:  https://github.com/Car-eye-admin
 * Copyright car-eye 车辆管理平台  2018 
 */


 





#ifndef _SVR_API_
#define _SVR_API_

#ifdef __cplusplus
extern "C"{
 #endif
 
u32 SVR_DataMessage(u8 *data, u32 datalen);
u32 SVR_DataMessageEx(u8 *data, u32 datalen, u32 timeflag);
u8 SVR_FrameSend(u8 *data, u32 datalen);
u8 SVR_FrameSendEx(u8 *data, u32 datalen, u32 time);//数据透传到服务器
u8 SVR_FrameSendExEx(u8 *data, u32 datalen, u32 time);//外部接口制定时间戳
u8 SVR_FrameRead(u8 *cmd1, u8 *cmd2, u8 **dataread, u32 *datalen);
u8 SVR_FrameSend_Mem(u8 *data, u32 datalen, u32 time);
u8 SVR_Cmd(u8 cmd1, u8 cmd2, u8 *data, u32 datalen);
u8 SVR_Cmd12(u8 cmd2, u8 *data, u32 datalen);
u8 SVR_Cmd11(u8 cmd2, u8 *data, u32 datalen);
u8 SVR_Cmd84(u8 type);
u8 SVR_cmdxx_Ex(u8 *cmd, u32 cmdlen, u32 timeflag, u8 cmd1);
u32 SVR_LicenseGet(u8 *cmd, u32 cmdlen, u32 time);

u8 SVR_heard(void);
u8 SVR_logon(void);
u8 SVR_enter(void);
u8 SVR_rout_save(u32 starttime, u32 distance, u32 fuel);
u8 SVR_routEx(void);
u8 SVR_mobileinforGet(void);
u16 SVR_Bigmem_send(void);
u8 SVR_system_synchron(void);
u32 SVR_gps_online(u8 *gpsdata, u32 lat, u32 log);
u32 SVR_gps_onlineEx(u8 *gpsdata, u32 lat, u32 log);
u8 SVR_SvrChange(u8 *addr, u32 port);
void SVR_SMSdeal(u8 *data);

u8 SVR_FameDeal(u8 *framedata, u32 datalen);
u8 G_system_time_synchron(void);

u8 SVR_back7f(u8 cmdsub, u8 status);
u32 ydmhms2u32(u32 ymd, u32 time);

void G_system_time_Set(u32 ymd, u32 time);
u8 G_system_time_SetExEx(unsigned int time);
u32 G_system_time_getEx(void);
void SVR_test(u8 type);

#ifdef __cplusplus
}
#endif
#endif
  

