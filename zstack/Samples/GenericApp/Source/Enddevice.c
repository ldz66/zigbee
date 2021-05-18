#include "OSAL.h"
#include "AF.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDProfile.h"
#include <string.h>
#include "Coordinator.h"
#include "DebugTrace.h"
#if !defined( WIN32)
#include "OnBoard.h"
#endif
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_uart.h"

const cId_t GenericApp_ClusterList[GENERICAPP_MAX_CLUSTERS ] = 
{
    GENERICAPP_CLUSTERID
};

const SimpleDescriptionFormat_t GenericApp_SimpleDesc = 
{
    GENERICAPP_ENDPOINT,
    GENERICAPP_PROFID,
    GENERICAPP_DEVICEID,
    GENERICAPP_DEVICE_VERSION,
    GENERICAPP_FLAGS,
    0,
    (cId_t *)NULL,
    GENERICAPP_MAX_CLUSTERS,
    (cId_t *)GenericApp_ClusterList
};
//定义四个变量
endPointDesc_t GenericApp_epDesc;//节点描述符
byte GenericApp_TaskID;//任务优先级
byte GenericApp_TransID;//数据发送序列号
devStates_t GenericApp_NwkState;//保存节点状态

//函数声明
void GenericApp_MessageMSGCB( afIncomingMSGPacket_t *pckt);//消息处理函数
void GenericApp_SendTheMessage(void);//数据发送函数

//以下为任务初始化函数
/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
* 函数名  ：GenericApp_Init
* 参数    ：byte task_id
* 返回    ：void
* 作者    ：daiyuhan
* 时间    ：2021/5/18
* 描述    ：函数说明
----------------------------------------------------------------*/
void GenericApp_Init( byte task_id )
{
    GenericApp_TaskID   =   task_id;//初始化任务优先级
    GenericApp_NwkState   =   DEV_INIT;//将设备状态初始化为DEV_INIT，表示该节点没有连接到Zigbee网络
    GenericApp_TransID    =   0;//将数据包序号初始化为0
    GenericApp_epDesc.endPoint    =   GENERICAPP_ENDPOINT;
    GenericApp_epDesc.task_id   =   &GenericApp_TaskID;
    GenericApp_epDesc.simpleDesc    =   
      (SimpleDescriptionFormat_t *)&GenericApp_SimpleDesc;
    GenericApp_epDesc.latencyReq    =   noLatencyReqs;//对节点描述符进行初始化
    afRegister( &GenericApp_epDesc );//使用afRegister函数将节点描述符进行注册
}

//消息处理函数，完成对接收数据的处理
/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
* 函数名  ：GenericApp_ProcessEvent
* 参数    ： byte task_id, UINT16 events 
* 返回    ：UINT16
* 作者    ：daiyuhan
* 时间    ：2021/5/18
* 描述    ：函数说明
----------------------------------------------------------------*/
UINT16 GenericApp_ProcessEvent( byte task_id, UINT16 events )
{
  afIncomingMSGPacket_t *MSGpkt;
  if ( events &SYS_EVENT_MSG )
  {
     MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive
       (GenericApp_TaskID);
     while ( MSGpkt )
     {
        switch ( MSGpkt->hdr.event )
        {
        case ZDO_STATE_CHANGE:
          GenericApp_NwkState = (devStates_t)(MSGpkt->hdr.
          status);//读取节点设备类型
          if (GenericApp_NwkState == DEV_END_DEVICE)//对节点设备类型进行判断，是终端节点，继续执行，实现无线数据发送
          {
            GenericApp_SendTheMessage() ;
          }
          break;
        default:
          break;
        }
        osal_msg_deallocate( (uint8 *)MSGpkt );
        MSGpkt = (afIncomingMSGPacket_t*)osal_msg_receive(GenericApp_TaskID );
}
        return (events ^ SYS_EVENT_MSG);
  }
         return 0;
     }
        void GenericApp_SendTheMessage( void )
        {
          unsigned char theMessageData[4] = "LED";//定义数组，存放要发送的数据
          afAddrType_t my_DstAddr;//定义变量
          my_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;//将发送地址模式设置为单播（Addr16Bit）
          my_DstAddr.endPoint = GENERICAPP_ENDPOINT;//初始化端口号
          my_DstAddr.addr.shortAddr = 0x0000;//0x0000为协调器网络地址，在Zigbee网络中是固定的
          AF_DataRequest( &my_DstAddr,&GenericApp_epDesc,//调用数据发送函数AF_DataRequest进行无线数据发送
                         GENERICAPP_CLUSTERID,
                          3,
                          theMessageData,
                          &GenericApp_TransID,
                          AF_DISCV_ROUTE, 
                          AF_DEFAULT_RADIUS);
              HalLedBlink (HAL_LED_2,0,50,500);//调用HalLedBlink函数，使终端节点的LED2闪烁
        }