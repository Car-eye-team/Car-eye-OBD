/* car-eye车辆管理平台 
 * car-eye车辆管理公共平台   www.car-eye.cn
 * car-eye开源网址:  https://github.com/Car-eye-admin
 * Copyright car-eye 车辆管理平台  2018
 */

#ifndef _AT_API_
#define _AT_API_

#ifdef __cplusplus
extern "C"{
 #endif


/*记录SIM卡信息
*/
typedef struct{
	u8 tellen;//电话号码字节个数
	u8 tel[12];//电话号码 超过11个字符只取11个
	u8 ccidlen;//ccid字节个数
	u8 ccid[21];//ccid  超过20个字符只取20个
	//ccid解析信息
	u8 tsp;//运营商代号
	       /*0 = 未知道
	         1 = 移动
	         2 = 联通
	         3 = 电信
	       */
	u8 business;//业务接入号  ccid.7
	u8 simfun;//SIM卡功能 一般为0，预付费SIM卡为1
	u8 province;//各省编码
	u8 year;//年号
	u8 supplier;//供应商代号
}_SIMinf;

/*记录M2M信息
*/
typedef struct{
	u8 imeilen;
	u8 imei[24];
	u8 cimilen;
	u8 cimi[24];
	//cimi解析信息
	u16 mmc;//移动国家号码
	u8 mnc;//移动网号
}_M2Minf;



/*记录运行状态
*/
typedef struct{
	u8 modemstart;//modem是否已经启动 0 = 未启动 1=已启动
	u8 gprsbond;//GPRS附着 0 = 未附着 1=附着
	u8 gprslink;//GPRS连接 0 = 未连接 1=已连接
	u8 csq;
	u8 m2mstatus;
	u8 localiplen;
	u8 localip[25];
	u8 seraddr[128];//服务器地址，支持IP以及域名 长度不超过128个字符
	u32 port;
}_M2Mstatus;

void AT_Init(u8 *seraddr, u32 port);
/*M2M检测*/ 
u8 AT_AT(void);
u8 AT_CPIN(void);

//软复位
u8 AT_CFUN(u8 status);

//设置
u8 AT_EQV(void);

//获取M2M信息
u8 AT_GSN(u8 *IMEI);
u8 AT_CIMI(u8 *CIMI);
u8 AT_CCID(u8 *CCID);
u8 AT_CNUM(u8 *num);

//GPRS
u8 AT_CREG(void);
u8 AT_CGATT(void);
u8 AT_CGREG(void);
u8 AT_CIICR(void);
u8 AT_CSQ(void);
u8 AT_CSTT(u8 flag);
u8 AT_CIFSR(u8 *ip);
u8 AT_CIPSTART(u8 *ip, u32 port);
u8 AT_CIPSHUT(void);
u8 AT_CIPCLOSE(void);
u8 AT_CIPSEND(u8 *data, u16 datalen);
u8 AT_CENG(void);
u8 AT_CMGDA(void);
u8 AT_SMSENDex(u8 *tell, u8 *data);
u8 AT_SMSinit(void);
//SMS设置
u8 AT_CNMI(void);

//应用接口
void m2m_startcheck(void);
void sim_information(void);
void m2m_information(void);
void m2m_gprsbond(void);
void m2m_gprslink(void);
u8 * m2m_imeiget(void);
//状态信息
u8 m2m_status_modemstart(void);
u8 m2m_status_gprsbond(void);
u8 m2m_status_gprslink(void);
u8 m2m_status(void);
void m2m_statusSet(u8 status);

u8 m2m_svr_addrtemplerset(u8 *addr, u32 port);
u8 m2m_svr_addrreback(void);
u8 m2m_ser_addrcheck(u8 *addr);
void AT_TTS(char *mus);
void AT_CREC(char *file, u8 mu);

unsigned char UDP_Creat(void);
void UDP_Cipclose(void);
unsigned char UDP_Send(unsigned char *send, int sendlen);

void UDP_Cipclose(void);
u8 AT_CIPSEND_test(u8 *data, u16 datalen);
u8 AT_CIPSTART_test(u8 *ip, u32 port);
u8 AT_CIPCLOSE_test(void);

u32 AT_VER(void);
u8 *m2m_verget(void);
u32 m2m_volget(void);
u8 *m2m_ccidget(void);
void m2m_ccidread(void);
u8 AT_CADC(void);
u32 AT_CBC(void);
void AT_ATH(void);
void AT_ATA(void);

void AT_CENG_CELL(u32 *lac, u32 *cell);
#ifdef __cplusplus
}
#endif
#endif
  

