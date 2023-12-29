#include "platform.h"
#include "vmnet.h"
#include "mqtt.h"
#include "vmdebug.h"

#define     D_MOD_BROKER    "mqttBroker"
typedef struct _st_mqttBroker
{
    PCHAR   psz_Cert;
    u32     u32_CntClient;
    u32     u32_CntClientActive;
    u32     u32_CntTopic;
    u32     u32_CntSent;
    u32     u32_CntRecv;
} ST_MQTT_BROKER, * PST_MQTT_BROKER;

ST_MQTT_BROKER  gst_MqttBroker = {0};

int         mqttConnectionHdl(PVOID pv_Ref, HANDLE h_Client, VMNET_EV e_Event, PVOID pv_Param, u32 u32_Params)
{
    ST_MQTT_MSSG    st_Mssg = {0};

    switch(e_Event)
    {
        case VMNET_EV_INC_CLIENT:
            break;

        case VMNET_EV_NET_ERROR:
            VM_PRINT_LOG((D_MOD_BROKER, E_VMDEBUG_WARN, "socket closed from destination\n"));

            if ( !mqttTractDisconnect(h_Client,&st_Mssg))
            {
                gst_MqttBroker.u32_CntClientActive--;
            }
            break;

        case VMNET_EV_PAYL_RECEIVE:
            VM_PRINT_DBG((D_MOD_BROKER, E_VMDEBUG_VERBOSE, "network packet recv. size %d\n", (u16)u32_Params));
          
            st_Mssg.pu8_Buf     = (u8 *)pv_Param;
            st_Mssg.u32_BufLen  = u32_Params;
            st_Mssg.u32_Cur     = 0;

            while ( ! mqttFixedHeaderParse(&st_Mssg) )
            {
                switch(st_Mssg.e_Cmd)
                {
                    case E_MQTT_CMD_CONNECT:
                       if ( !mqttTractConnect(h_Client,&st_Mssg) )
                       {
                            gst_MqttBroker.u32_CntClientActive++;
                            gst_MqttBroker.u32_CntClient++;
                       }
                        break;

                    case E_MQTT_CMD_PING_REQ:
                        mqttPingResponseTransmit(h_Client);
                        break;

                    case E_MQTT_CMD_SUBSCRIBE:
                        mqttTractSubscribe(h_Client,&st_Mssg);
                        VM_PRINT_DBG((D_MOD_BROKER, E_VMDEBUG_INFO, "Num of subscribed topics %d\n", topicStatisticGet()));
                        break;
                    case E_MQTT_CMD_PUBLISH:
                        VM_PRINT_DBG((D_MOD_BROKER, E_VMDEBUG_INFO,"publish mssg\n"));

                        mqttTractPublish(h_Client, &st_Mssg);
                    break;

                    case E_MQTT_CMD_DISCONNECT:
                        VM_PRINT_DBG((D_MOD_BROKER, E_VMDEBUG_INFO,"destination has disconnected!\n"));
                        if ( !mqttTractDisconnect(h_Client,&st_Mssg))
                        {
                            gst_MqttBroker.u32_CntClientActive--;
                        }

                        return D_VMNET_SOCKET_CLOSING;
                    default:
                    break;
                }
            }
            
            break;
        default:
        break;
    }

    return 0;
}

int         mqttBrokerStart(PCHAR psz_IP, u16 u16_Port, PCHAR psz_Certificate)
{
    printf("[mqttBroker] INFO Broker start on %s:%d with %s\n", psz_IP, u16_Port, psz_Certificate);
    topicItemListInitialize();
    vmsrvStart(&gst_MqttBroker, psz_IP, u16_Port, (PFN_NOTIFY)mqttConnectionHdl);
}