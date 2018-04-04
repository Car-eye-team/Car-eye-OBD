/* car-eye车辆管理平台 
 * car-eye车辆管理公共平台   www.car-eye.cn
 * car-eye开源网址:  https://github.com/Car-eye-admin
 * Copyright car-eye 车辆管理平台  2018
 */


#ifndef __DB__
#define __DB__

#ifdef __cplusplus
extern "C"{
 #endif
#define UPDATE_FILE_MAX 6
typedef struct{
   u8 type;
   u32 size;
   u32 cs;
   u8 name[32];//文件名不超过32个字节
}_updatefile;
typedef struct{
  u8 filenum;
  _updatefile file[UPDATE_FILE_MAX];
}_update;

/*OBD数据备份，防止OBD拔出数据丢失
*2015/6/8 6:31  fangcuisong
*/
typedef struct{
	u32 engine_on_time_135;//总运行时间
	u32 engine_on_time_136;//短期运行时间
	u32 fuel_used_138;//总油耗
	u32 fuel_used_139;//短期油耗
	u32 fuel_used_av_141;//总平均油耗
	u32 fuel_used_av_142;//短期平均油耗
	u32 dist_147;//总里程
	u32 dist_148;//短期里程
	u32 gpstime;
	u32 gpslat;//纬度
	u32 gpslon;//经度
}_OBD_DB;
//2015/6/8 6:47
u8 db_obd_init(void);
u8 db_obd_insert(u8 id, u32 data);
u8 db_obd_save(void);
u8 db_obd_reset(void);
/*升级文件类型定义
*/
#define UPDATE_OBD_APP 12
#define UPDATE_OBD_CAN 2
#define UPDATE_MUSIC 3
#define UPDATE_OBD_LICENSE 4
#define UPDATE_M2M_APP 16
#define UPDATE_M2M_LICENSE 6

u8 db_init(void);
u8 db_update_init(void);
u8 db_update_save(u8 type, u8 *name, u32 size, u32 cs);
u8 *db_update_fileget(u8 type);
/*****************************************************************/
//记录服务器信息
typedef struct{
   u8 svraddr[128];//服务器地址 可以是IP 也可以为域名
   u32 port;//服务器端口号
   u8 svraddr1[128];//辅助服务器地址IP
   u32 port1;//辅助服务器端口号
   u8 svrtell_t[24];//短信服务器发送号码
   u8 svrtell_r[24];//短信服务器接收号码
   u8 apn[16];//APN
   u8 ttell[32];//用于拔插测试的短信号码
   u8 telx[5][24];//用户号码
   u16 backcircle;//数据返回周期 S为单位，0表示默认由设备决定
   u8 media;//多媒体播放使能开关  0=禁止  1=使能 默认使能
 }_SVR;
 
 /*GPS数据
 */
 typedef struct{
 	u32 lac;
 	u32 lon;
 	u32 cellid;
 	u32 localid;
}_DBGPS;
 /*二级缓存数据记录文件列表
 *每个文件最大100K
 *最多10个文件 最多1M的数据存储
 *****************************************************/
 #define DB_FILE_MAX 10
 typedef struct{
    u8 filenum;//有效文件数
    u32 filesize[DB_FILE_MAX];//每个文件大小
 }_DBFILE;
 void db_fileinit(void);
 u32 db_fileread(u8 *data);
 u8 db_filesave(u8 *data, u32 datalen, u8 flag);
 u8 db_filecheck(void);
 
 u8 db_svr_default(void);
 u8 db_svr_save(void);
 u8 db_svr_init(void);
 u32 db_svr_cyclget(void);
 void db_svr_cyclset(u32 cycl);
 u32 db_svr_portget(void);
 void db_svr_portset(u32 port);
 u32 db_svr_port1get(void);
 void db_svr_port1set(u32 port);
 u8 *db_svr_sttelget(void);
 void db_svr_sttelset(u8 *stel);
 void db_svr_srtelset(u8 *stel);
 u8 *db_svr_srtelget(void);
 u8 *db_svr_addrget(void);
 u8 *db_svr_addr1get(void);
 void db_svr_addrset(u8 *addr);
 void db_svr_addr1set(u8 *addr);
 void db_svr_telxset(u8 *tel, u8 telnum);
 
 u16 rout_fileread(u8 **dataptr);
 u8 rout_filesave(u8 *dataptr, u32 datalen);
 
u8 db_update_vercheck(u16 ver, u8 type);

//GPS数据
u8 db_gps_init(void);
u8 db_gps_get(u32 *lac, u32 *loc);
u8 db_gps_save(void);
void db_gps_set(u32 lac, u32 loc);

u8 db_save(u8 *file, u32 filesize, u8 *filename);

void db_svr_ttellset(u8 *ttell);
u8 *db_svr_ttellget(void);
void db_svr_apnset(u8 *apn);
u8 *db_svr_apnget(void);

u8 db_svr_mmcget(void);
void db_svr_mmcset(u8 flag);
u8 db_swver_save(u8 *infor);
void db_obd_gpsclr(void);
void db_obd_gpsset(u32 gpstime, u32 gpslat, u32 gpslon);
void db_gps_cellsave(void);
u8 db_gpscell_get(u32 *lac, u32 *cel);

u8 db_svr_adcwarmget(void);
void db_svr_adcwarmset(u8 flag);
#ifdef __cplusplus
}
#endif
#endif



