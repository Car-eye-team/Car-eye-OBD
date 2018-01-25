/* car-eye车辆管理平台 
 * car-eye车辆管理公共平台   www.car-eye.cn
 * car-eye开源网址:  https://github.com/Car-eye-admin
 * Copyright car-eye 车辆管理平台  2017 
 */
#ifndef H_CAN_DRIVER_H
#define H_CAN_DRIVER_H

/********************************************************************************
* Include header
********************************************************************************/
//#include "stm32f10x.h"

/********************************************************************************
* Types definition
********************************************************************************/

typedef enum {
CAN_BAUD_125K=0, 
CAN_BAUD_250K=1, 
CAN_BAUD_500K=2
}CAN_BAUD;

typedef enum {
CAN_MODE_NORMAL=0,
CAN_MODE_LOOPBACK=1,
CAN_MODE_SILENT=2,
CAN_MODE_SILENT_LOOPBACK=3
}CAN_MODE;

typedef enum {
RULE_RECEIVE_ALL=0, 
RULE_ID_LIST=1, 
RULE_ID_MASK
}CAN_FILTER_RULE;

/********************************************************************************
* Types define
********************************************************************************/
//Set ring buffer size for receive
#define CAN_RX_RING_SIZE 32	

//Set ring buffer size for transmit
#define CAN_TX_RING_SIZE 8 

/********************************************************************************
* Public Function
********************************************************************************/
//Init CAN controller & corresponding hardware
void Driver_CAN_Init(void);	

//Uninit CAN controller & corresponding hardware
void Driver_CAN_Uninit(void);

//Put a CAN frame to transmit ring buffer
bool Driver_CAN_Send(u8 ext, u8 remote, u32 canid, u8 datalen, u8 * data);	

//Get a CAN frame from receive ring buffer
bool Driver_CAN_Receive(u8 * ext, u8 * remote, u32* canid, u8* datalen, u8* data);

//mode can be any value defined in CAN_MODE
void Driver_CAN_SetMode(CAN_MODE mode); 

//Set CAN baud rate: CAN_BAUD_125K, CAN_BAUD_250K, CAN_BAUD_500K
void Driver_CAN_SetBaud(CAN_BAUD baud);

//Clear receive ring and clear all receive FIFO.
void Driver_CAN_ClearRxBuffer(void);	

//Clear transmit ring and cancel all task of transmit msgbox 
void Driver_CAN_ClearTxBuffer(void);	

//Set CAN receive filter rule: RULE_RECEIVE_ALL, RULE_ID_LIST
void Driver_CAN_SetFilterRule(CAN_FILTER_RULE rule,u32 Receive_ID,u16 MaskIdHigh,u16 MaskIdLow);

//Append a CAN id to filter list, under IdList rule mode
bool Driver_CAN_FilterIdListAdd(u8 ext, u8 remote, u32 canid);

//Clear all CAN id from filter list
void Driver_CAN_FilterIdListClear(void);


/********************************************************************************
* Private Marcros
********************************************************************************/
#define CAN1_FILTER_FIRST_ID	1
#define CAN1_FILTER_COUNT		28

/********************************************************************************
* Private Function
********************************************************************************/
void Driver_CAN_UpdateParameter(void);
bool Driver_CAN_AddIdList32(u32 id, u8 isremote);
bool Driver_CAN_AddIdList16(u16 id, u8 isremote);
void USB_HP_CAN1_TX_IRQHandler(void);
void USB_LP_CAN1_RX0_IRQHandler(void);
void CAN1_RX1_IRQHandler(void);


#endif
