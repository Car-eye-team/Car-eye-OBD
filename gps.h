/* car-eye车辆管理平台 
 * car-eye车辆管理公共平台   www.car-eye.cn
 * car-eye开源网址:  https://github.com/Car-eye-admin
 * Copyright car-eye 车辆管理平台  2018
 */
#ifndef __gps__
#define __gps__

#ifdef __cplusplus
extern "C"{
#endif
#define GPS_DATA_MAX 50
#define GPS_DATA_longflag_id 156  //经度
#define GPS_DATA_longitude_id 157
#define GPS_DATA_latflag_id 158//纬度
#define GPS_DATA_latitude_id 159
#define GPS_DATA_angle_id 163
#define GPS_DATA_celid_id 160
#define GPS_DATA_lacid_id 161
typedef struct{
	u8 num;
	u8 id[16];//最多支持16条数据
	u32 data[16];
}_OBDGPS;//OBD数据放到GPS中


typedef struct{
	_OBDGPS obddata;
  u8 enable;//标志GPS是否有效 根据GPRMC的定位状态表示数据是否可用 0x55=可用
  u32 latitude;//纬度 格式:ddmm.mmmm *10000
  u8  latflag;//1=北纬N  2=南纬S
  u32 longitude;//经度  格式:dddmm.mmmm  10000 = °
  u8  longflag;//1=东经E 2=西经W
  u32 speed;//速度 /100 = Km/h
  u32 angle;//角度 /100 = °
  u32 ymd;//年月日
  u32 time;//时间
  u32 timeflag;//时间戳
  u32 distance;//本次里程
  u32 fuel;//本次油耗
  u32 celid;//小区号
  u32 lacid;//位置区号码
  u32 itemflag;//数据项读取标记 0=未读取 1=已读取
              /*
              bit0 = 时间
              bit1 = 日期
              bit2 = 经度
              bit3 = 纬度
              bit4 = 速度
              bit5 = 角度
              bit31 = GPS数据有效
              */
  u32 _3gcheck;//3急检测数据
   char Time808[6];  //add by lilei-2018-04-02
}_GPSUNIT;


typedef struct{
  _GPSUNIT unit[GPS_DATA_MAX];
  u8 oprate;//0x55=正在操作
  u8 num;
  u8 in;
  u8 out;
}_GPS;

//用于静态漂移算法
typedef struct{
    _GPSUNIT gpscur;
    u8  lathh;//纬度->度
    u8  latmm;//纬度->分
    double  latss;//纬度->秒
    u8  longhh;//经度->度
    u8  longmm;//经度->分
    double  longss;//经度->秒
    u32 vehiclespeed;//车速
}_GPSM;

 extern _GPSUNIT Lgpscur;
void gps_Init(void);
u8 gps_read(void);
void app_gps(void *data);
u8 gps_status_get(void);
u32 gps_speed_get(void);
u8 GPS_2MDM_writeEx(void);
u8 gps_tosvr(void);
u8 gps_data_transformEx(void);

u8 gps_gsmpositionset(u32 cellid, u32 lacid);
u8 gps_mobileinfor(u8 *data);
u8 gps_tobigmem(void);
u8 gps_data_get(u32 *lat, u32 *lon, u32 *speed, u32 *angle);
void gps_obdinsert(u8 id, u32 data);
void gps_vehicle_status_set(u8 flag);

u8 gps_wgs84_degrees(double latin, double lonin, double *latout, double *lonout);
void gps_3g_check_set(u8 flag);
u8 gps_assist_online(void);
u8 gps_assist_online_from_zgex(void);
u8 gps_assist_toOBD(void);
u32 gps_time_get(void);
u32 gps_data_read(u8 *data, u32 datalen);
void gps_demo(void);
void UtcToBeiJingTIme(char *TBuf);
#ifdef __cplusplus
}
#endif
#endif


