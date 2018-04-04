/* car-eye车辆管理平台 
 * car-eye车辆管理公共平台   www.car-eye.cn
 * car-eye开源网址:  https://github.com/Car-eye-admin
 * Copyright car-eye 车辆管理平台  2018
 */

#ifndef _APP_USER2_
#define _APP_USER2_

#ifdef __cplusplus
extern "C"{
#endif

/*应用时钟定义
*时钟来自三个地方：
*  1、系统内核  每次数据传输时更新
*  2、GPS       每次成功获取到GPS时间时更新
*  3、应用计算  每秒更新
*/
typedef struct{
 u32 time_rtc;
 u32 time_rtc_update;
 u32 time_gps;
 u32 time_gps_update;
 u32 time_app;
 u32 time_app_update;
}_stime;

void Lstime_gps_update(u32 time);
void Lstime_sys_update(u32 time);
u32 Lstime_get(void);

void app_linking(void *data);
void app_keep_step_set(u8 state);
void app_svrlink(u8 state);
void linking_init(void);

void app_sleep_flag_set(u8 flag);
#ifdef __cplusplus
}
#endif
#endif


