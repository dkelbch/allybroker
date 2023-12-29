#include "platform.h"
#include "vmdebug.h"
#include "vmnet.h"
#include "mqtt.h"

#define     D_MOD_MQTT_TRACT    "mqttTract"

ST_MQTT_CLIENT      gst_mqttClient[D_MQTT_CLIENT_NUM]={0};

static      PST_MQTT_CLIENT         _mqttClientNew(int h_Client, PCHAR psz_ClientId)
{
    unsigned int    i;

    for(i=0;i < sizeof(gst_mqttClient)/sizeof(ST_MQTT_CLIENT);i++)
    {
        if ( !gst_mqttClient[i].h_Connection)
        {
            gst_mqttClient[i].h_Connection = h_Client;

            if ( psz_ClientId)
            {
                memcpy( gst_mqttClient[i].u8_ClientId, (u8*)psz_ClientId, MIN(strlen(psz_ClientId),sizeof(gst_mqttClient[i].u8_ClientId) ));
            }

            VM_PRINT_LOG((D_MOD_MQTT_TRACT, E_VMDEBUG_VERBOSE, "new client on slot %d with hdl %d\n", i, h_Client));

            return gst_mqttClient + i;
        }
    }

    VM_PRINT_LOG((D_MOD_MQTT_TRACT, E_VMDEBUG_ERR, "no client slot free!\n"));

    return VM_NULL;
}

static      PST_MQTT_CLIENT         _mqttClientFind(int h_Connection)
{
    unsigned int    i;
    for(i=0; i < sizeof(gst_mqttClient)/sizeof(ST_MQTT_CLIENT);i++)
    {
        if( gst_mqttClient[i].h_Connection == h_Connection)
        {
            return gst_mqttClient + i;
        }
    }
    return VM_NULL;
}

static      void                    _mqttClientDel(PST_MQTT_CLIENT pst_Client)
{
    VM_PRINT_LOG((D_MOD_MQTT_TRACT, E_VMDEBUG_INFO, "free client on slot with hdl %d\n", pst_Client->h_Connection));
    pst_Client->h_Connection = 0;
}
/*************************************************
 * 
 * Connection establishment
 * 
 *************************************************/

/*!
    \brief  handle connect request from the client
    \param[in]      h_Client    socket handle of client
    \param[in,out]  pMssg       pointer to message object
    \return         E_MQTT_INT_CODE
*/
E_MQTT_INT_CODE         mqttTractConnect(int h_Client, PST_MQTT_MSSG pMssg)
{
    E_MQTT_INT_CODE n_Ret = E_MQTT_INT_CODE_OK;

    //parse connect options and return MQTT_CONNECT_ACK
    if( !mqttCmdConnectParse(pMssg))
    {
        ST_MQTT_MSSG_CONACK     st_ConAck ={E_MQTT_CONACK_RET_CODE_ACCPT,0};
        if ( _mqttClientFind(h_Client) == VM_NULL)
        {
            if ( _mqttClientNew(h_Client, (PCHAR)"test_client") == VM_NULL)
            {
                st_ConAck.e_RetCode = E_MQTT_CONACK_RET_CODE_SRV_UNAVAIL;
            }
        
            mqttConnectAckTransmit(h_Client,&st_ConAck);
            n_Ret = E_MQTT_INT_CODE_CLOSE;
        }
        else
        {
            //disconnect
            n_Ret = E_MQTT_INT_CODE_CLOSE;
        }
    }
    else
    {
       // VM_LOG_ERR(("[mqttProt] ERR Connect cmd"))
       VM_PRINT_LOG((D_MOD_MQTT_TRACT, E_VMDEBUG_ERR, "connect cmd failed!\n"));
       
       n_Ret = E_MQTT_INT_CODE_NOT_FIND;
    }

    return n_Ret;
}

E_MQTT_INT_CODE         mqttTractDisconnect(int h_Client, PST_MQTT_MSSG pMssg)
{
    E_MQTT_INT_CODE n_Ret = E_MQTT_INT_CODE_OK;

    UNUSED(pMssg);

    PST_MQTT_CLIENT pClient = _mqttClientFind(h_Client);
    if ( pClient )
    {
        top2subscriptionListClientRemove(pClient);

        _mqttClientDel(pClient);
    }
    else
    {
        n_Ret = E_MQTT_INT_CODE_NOT_FIND;
    }

    return n_Ret; 
}

E_MQTT_INT_CODE         mqttTractSubscribe(int h_Client, PST_MQTT_MSSG pMssg)
{
    E_MQTT_INT_CODE n_Ret = E_MQTT_INT_CODE_OK;

    PST_MQTT_CLIENT pClient = _mqttClientFind(h_Client);
    if ( pClient )
    {

        if ( !mqttCmdSubscribeParse(pMssg))
        {
            // QoS needs to be managed, later.
            if ( !topic2subcriptionListAdd(pMssg->psz_Topic, pClient) )
                mqttSubscribeAckTransmit(h_Client, pMssg->u16_MssgId, 0);
        }
    }
    else
    {
         VM_PRINT_LOG((D_MOD_MQTT_TRACT, E_VMDEBUG_ERR, "subscribe cmd failed!\n"));
       
         n_Ret = E_MQTT_INT_CODE_NOT_FIND;
    }

    return n_Ret;
}

E_MQTT_INT_CODE         mqttTractPublish(int h_Client, PST_MQTT_MSSG pMssg)
{
    E_MQTT_INT_CODE n_Ret = E_MQTT_INT_CODE_OK;

    UNUSED(h_Client);

    if( !mqttCmdPublishParse(pMssg)) 
    {
        topicPublishForwarder(pMssg);
        //mqttPublishAckTransmit(h_Client, pMssg->u16_MssgId);
    }

    return n_Ret;
}

E_MQTT_INT_CODE         mqttTractMssgForward(int h_Connection, u8* pu8_Buf, u16 u16_BufSize)
{
    //later exchage MssgID
    printf("handle %d, Mssg out-> size %d\n", h_Connection, u16_BufSize);
    VM_PRINT_DBG((D_MOD_MQTT_TRACT, E_VMDEBUG_INFO, "on client hdl %d mssg out -> size %d\n", h_Connection, u16_BufSize));
    return vmsrvBufferTransmit(h_Connection, pu8_Buf, u16_BufSize);
}

//*/