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
};//���豸������
//������������
endPointDesc_t GenericApp_epDesc;//�ڵ�������GenericApp_epDesc
byte GenericApp_TaskID;//�������ȼ�GenericApp_TaskID
byte GenericApp_TransID;//���ݷ������к�GenericApp_TransID



void GenericApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );//������Ϣ������
void GenericApp_SendTheMessage( void );//���ݷ��ͺ���
/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

* ������  ��GenericApp_Init
* ����    ��byte byte task_id
* ����    ��void
* ����    ��zhangyuchen
* ʱ��    ��2021/5/17
* ����    ������˵��
----------------------------------------------------------------*/

void GenericApp_Init( byte task_id )//��ʼ������
{
    GenericApp_TaskID   = task_id;//��ʼ�����������ȼ�
    GenericApp_TransID   = 0;//���������ݰ�����ų�ʼ��Ϊ0
    GenericApp_epDesc.endPoint = GENERICAPP_ENDPOINT;
    GenericApp_epDesc.task_id = &GenericApp_TaskID;
    GenericApp_epDesc.simpleDesc =
        (SimpleDescriptionFormat_t *)&GenericApp_SimpleDesc;
    GenericApp_epDesc.latencyReq = noLatencyReqs;//�Խڵ����������г�ʼ��
    afRegister( &GenericApp_epDesc );//ʹ��afRegister�������ڵ�����������ע��
}
/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

* ������  ��GenericApp_ProcessEvent
* ����    �� byte tasak_id, UINT15 events 
* ����    ��UINT16
* ����    ��Lishuqi
* ʱ��    ��2021/5/18
* ����    ������˵��
----------------------------------------------------------------*/
UINT16 GenericApp_ProcessEvent( byte task_id, UINT16 events )
{
    afIncomingMSGPacket_t *MSGpkt;//������һ��ָ�������Ϣ�ṹ���ָ��MSGpkt
    if ( events & SYS_EVENT_MSG )
    {
        MSGpkt =(afIncomingMSGPacket_t *)osal_msg_receive(GenericApp_TaskID);
        //ʹ��osal_msg_receive����ָ����յ����������ݰ���ָ��
        while (MSGpkt)
        {
            switch ( MSGpkt->hdr.event )
            {
            case AF_INCOMING_MSG_CMD://�жϴ���
                GenericApp_MessageMSGCB( MSGpkt );
                break;
            default:
                break;
            }
            osal_msg_deallocate( (uint8 *)MSGpkt );//���յ�����Ϣ���������Ҫ�ͷ���Ϣ��ռ�ݵĴ洢�ռ�
            MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( GenericApp_TaskID );//������һ����Ϣ���ٴ���Ϣ�����������Ϣ��Ȼ����ֱ��������Ϣ������Ϊֹ
        }
        return (events ^ SYS_EVENT_MSG);
    }
    return 0;
}//������������Ϣ������

/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

* ������  ��GenericApp_MessageMSGCB
* ����    ��void
* ����    ��afIncomingMSGPacket_t *pkt
* ����    ��Lishuqi
* ʱ��    ��2021/5/18
* ����    ������˵��
----------------------------------------------------------------*/

void GenericApp_MessageMSGCB( afIncomingMSGPacket_t *pkt )
{
    unsigned char buffer[4] = " ";
    switch ( pkt->clusterId )
    {
    case GENERICAPP_CLUSTERID:
        osal_memcpy(buffer,pkt->cmd.Data,3);//���յ������ݿ�����������buffer��
        if((buffer[0] =='L')||(buffer[1] =='E')||(buffer[2]=='D'))//�жϽ��յ����ǲ��ǡ�LED�������ַ�����ִ����һ�У�������������LED2
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