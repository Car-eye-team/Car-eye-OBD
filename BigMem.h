/* car-eye车辆管理平台 
 * car-eye车辆管理公共平台   www.car-eye.cn
 * car-eye开源网址:  https://github.com/Car-eye-admin
 * Copyright car-eye 车辆管理平台  2018
 */


#ifndef _BIG_MEM_
#define _BIG_MEM_

#ifdef __cplusplus
extern "C"{
#endif
#define BIG_MEM_MSG_MAX 1280  //可以保存将近35分钟的OBD+GSP数据
#ifndef DATA_LOG_FILE_MAX
   #define DATA_LOG_FILE_MAX 1024*100 //车辆数据缓存文件大小
#endif
#ifndef BIG_MEM_MAX
    #define BIG_MEM_MAX 1024 * 150//150K
#endif
typedef struct{
  u32 index;
  u16 len;
}_BIGMSG;
typedef struct{
   u32 dataindex;//保存的数据序号 也是缓冲区数据大小
   u32 datalen;//缓冲区中的数据量
   u16 msgin;//写入帧序号
   u16 msgout;//读出帧序号
   u16 msgnum;//帧数量
   _BIGMSG msg[BIG_MEM_MSG_MAX];//每帧信息
 }_BIGMEM;
 
void bigmem_init(void);

u8 *bigmem_get(u8 type);
void bigmem_free(void);

u8 bigmem_obdgps_in(u8 *datain, u16 datalen);
u16 bigmem_obdgps_out(u8 *dataout);
u8 bigmem_obdgps_tosvr(void);
u8 bigmem_save(void);
#ifdef __cplusplus
}
#endif
#endif

