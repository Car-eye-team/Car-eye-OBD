/* car-eye车辆管理平台 
 * car-eye车辆管理公共平台   www.car-eye.cn
 * car-eye开源网址:  https://github.com/Car-eye-admin
 * Copyright car-eye 车辆管理平台  2018
 */

 
#if !defined (__APP_USER1__)
#define __APP_USER1__

#include "eat_interface.h"

//extern void app_time(void *data);
#define MDM_MSG_MAX 30
typedef struct{
   u16 MDMmsg_Datalen;
   u16 MDMmsg_Index;
 }_MDMmsg;
 typedef struct{
   u8 MDMmsgNum;
   u8 MDMmsgIn;
   u8 MDMmsgOut;
   _MDMmsg msg[MDM_MSG_MAX];
}_MDM;

/*广播数据结构
*/
#define BROAD_UNIT_MAX 48
typedef struct{
	u8 id;
	u32 data;
}_bdataunit;
typedef struct{
	u8 num;
	u8 unitesdone;//数据设置完成
	_bdataunit units[BROAD_UNIT_MAX];
}_bdata;
void Lbdata1_insert(u8 id, u32 datain);
void Lbdata2_insert(u8 id, u32 datain);
void Lbdata1_insert_done(void);
void Lbdata2_insert_done(void);

void mdm_Init(void);
u16 MDM_read(void);
u16 MDM_write(u8 *data, u16 datalen);
u16 MDM_write1(u8 *data, u16 datalen);
u16 MDM_DataToApp(u8 **dataptr);
u8 MDM_writeEnable(void);
u8 MDM_writeEx(void);

void user_meminit(void);
void *user_malloc(u32 size);
void *user_mfree(void *addr);
u32 user_time(void);
void MDM_unlinkset(u8 state);
u8 app_modem_status_get(void);


void svr_event30_enable(u8 state);
void svr_event01_enable(u8 state);
u8 m2m_obd_heard_set(u8 data);
u8 svr_event_mobileinfor_update(u8 state);
void Lobd_start_mode_set(unsigned char state);
#endif


