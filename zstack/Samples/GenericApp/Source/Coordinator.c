#include "OSAL.h"
#include "AF.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDProfile.h"
#include <string.h>
#include "Coordinator.h"
#include "DebugTrace.h"

#if !defined ( WIN32 )
#include "OnBoard.h"
#endif

#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_uart.h"

const cId_t GenericApp_ClusterList[GENERICAPP_MAX_CLUSTERS] = 
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
    GENERICAPP_MAX_CLUSTERS,
    (cId_t *)GenericApp_ClusterList,
    0,
    (cId_t *)NULL
};//简单设备描述符
//定义三个变量
endPointDesc_t GenericApp_epDesc;//节点描述符GenericApp_epDesc
byte GenericApp_TaskID;//任务优先级GenericApp_TaskID
byte GenericApp_TransID;//数据发送序列号GenericApp_TransID



void GenericApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );//声明消息处理函数
void GenericApp_SendTheMessage( void );//数据发送函数
/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

* 函数名  ：GenericApp_Init
* 参数    ：byte byte task_id
* 返回    ：void
* 作者    ：zhangyuchen
* 时间    ：2021/5/17
* 描述    ：函数说明
----------------------------------------------------------------*/

void GenericApp_Init( byte task_id )//初始化函数
{
    GenericApp_TaskID   = task_id;//初始化了任务优先级
    GenericApp_TransID   = 0;//将发送数据包的序号初始化为0
    GenericApp_epDesc.endPoint = GENERICAPP_ENDPOINT;
    GenericApp_epDesc.task_id = &GenericApp_TaskID;
    GenericApp_epDesc.simpleDesc =
        (SimpleDescriptionFormat_t *)&GenericApp_SimpleDesc;
    GenericApp_epDesc.latencyReq = noLatencyReqs;//对节点描述符进行初始化
    afRegister( &GenericApp_epDesc );//使用afRegister函数将节点描述符进行注册
}
/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

* 函数名  ：GenericApp_ProcessEvent
* 参数    ： byte tasak_id, UINT15 events 
* 返回    ：UINT16
* 作者    ：Lishuqi
* 时间    ：2021/5/18
* 描述    ：函数说明
----------------------------------------------------------------*/
UINT16 GenericApp_ProcessEvent( byte task_id, UINT16 events )
{
    afIncomingMSGPacket_t *MSGpkt;//定义了一个指向接收消息结构体的指针MSGpkt
    if ( events & SYS_EVENT_MSG )
    {
        MSGpkt =(afIncomingMSGPacket_t *)osal_msg_receive(GenericApp_TaskID);
        //使用osal_msg_receive函数指向接收到的无线数据包的指针
        while (MSGpkt)
        {
            switch ( MSGpkt->hdr.event )
            {
            case AF_INCOMING_MSG_CMD://判断代码
                GenericApp_MessageMSGCB( MSGpkt );
                break;
            default:
                break;
            }
            osal_msg_deallocate( (uint8 *)MSGpkt );//接收到的消息处理完后，需要释放消息所占据的存储空间
            MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( GenericApp_TaskID );//处理完一个消息后，再从消息队列里接收消息，然后处理，直到所有消息处理完为止
        }
        return (events ^ SYS_EVENT_MSG);
    }
    return 0;
}//上述代码是消息处理函数

/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

* 函数名  ：GenericApp_MessageMSGCB
* 参数    ：void
* 返回    ：afIncomingMSGPacket_t *pkt
* 作者    ：Lishuqi
* 时间    ：2021/5/18
* 描述    ：函数说明
----------------------------------------------------------------*/

void GenericApp_MessageMSGCB( afIncomingMSGPacket_t *pkt )
{
    unsigned char buffer[4] = " ";
    switch ( pkt->clusterId )
    {
    case GENERICAPP_CLUSTERID:
        osal_memcpy(buffer,pkt->cmd.Data,3);//将收到的数据拷贝到缓冲区buffer中
        if((buffer[0] =='L')||(buffer[1] =='E')||(buffer[2]=='D'))//判断接收到的是不是“LED”三个字符，是执行下一行，如果不是则点亮LED2
        {
            HalLedBlink(HAL_LED_2,0,50,500);
        }
        else
        {
            HalLedSet(HAL_LED_2,HAL_LED_MODE_ON);
        }
        break;
    }
}