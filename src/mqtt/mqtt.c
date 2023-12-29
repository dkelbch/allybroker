#include "platform.h"
#include "vmdebug.h"
#include "vmnet.h"
#include "mqtt.h"

#define         D_MOD_PROT_MQTT "protMqtt"

PCHAR       _mqttProtCmd2Str(E_MQTT_CMD e_Cmd)
{
    switch(e_Cmd)
    {
       caseretstr(E_MQTT_CMD_CONNECT);
       caseretstr(E_MQTT_CMD_CONNECT_ACK);
       caseretstr(E_MQTT_CMD_PUBLISH);
       caseretstr(E_MQTT_CMD_PUBLISH_ACK);
       caseretstr(E_MQTT_CMD_PUBLISH_REC);
       caseretstr(E_MQTT_CMD_PUBLISH_REL);
       caseretstr(E_MQTT_CMD_PUBLICH_CPLT);
       caseretstr(E_MQTT_CMD_SUBSCRIBE);
       caseretstr(E_MQTT_CMD_SUBSCRIBE_ACK);
       caseretstr(E_MQTT_CMD_UNSUBSCRIBE);
       caseretstr(E_MQTT_CMD_UNSUBSCRIBE_ACK);
       caseretstr(E_MQTT_CMD_PING_REQ);
       caseretstr(E_MQTT_CMD_PING_RSP);
       caseretstr(E_MQTT_CMD_DISCONNECT);
       default:
        break;
    }

    return(PCHAR)"unkown";
}
/*
Digits	From	To
1	0 (0x00)	127 (0x7F)
2	128 (0x80, 0x01)	16,383 (0xFF, 0x7F)
3	16,384 (0x80, 0x80, 0x01)	2,097,151 (0xFF, 0xFF, 0x7F)
4	2,097,152 (0x80, 0x80, 0x80, 0x01)	268,435,455 (0xFF, 0xFF, 0xFF, 0x7F)

*/
u32         _mqttProtocolLenParse(u8 * pu8_Buf, u32 * pu32_Len)
{
    int i=0;
    u32 u32_Multi=1;

    *pu32_Len = 0;
    
    do
    {
        //printf("digit %02x ", pu8_Buf[i]);
        if((pu8_Buf[i]&0x80) != 0x80 )
        {
            // only one digit;
            *pu32_Len += (u32)(pu8_Buf[i]&0x7F) * u32_Multi;
            //printf("-%d += %d\n", *pu32_Len,(u32)(pu8_Buf[i]&0x7F) * u32_Multi );

            return i+1;
        }
        u32_Multi *=128;
        i++;
    } while (i<4);

    return 0x7FFFFFFF;
}

E_MQTT_INT_CODE         mqttFixedHeaderParse( PST_MQTT_MSSG pMssg)
{
    VM_PRINT_DBG((D_MOD_PROT_MQTT, E_VMDEBUG_VERBOSE, "fixHdr: cur %d, total %d, bufsize %d\n", pMssg->u32_Cur, pMssg->u32_TotalLen, pMssg->u32_BufLen));
    
    if ( pMssg->u32_Cur < pMssg->u32_BufLen)
    {
        pMssg->e_Cmd        = D_MQTT_CMD_GET(pMssg->pu8_Buf[pMssg->u32_Cur]);
        pMssg->u8_Flags     = D_MQTT_FLAGS_GET(pMssg->u32_Cur);
        pMssg->u32_Cur++;
        pMssg->u32_Cur      += _mqttProtocolLenParse(pMssg->pu8_Buf+pMssg->u32_Cur, &pMssg->u32_TotalLen);

        if ( pMssg->u32_Cur >= 0x7FFFFFFF)
        {
            return E_MQTT_INT_CODE_ERR_PARSE;
        }
        VM_PRINT_DBG((D_MOD_PROT_MQTT,E_VMDEBUG_VERBOSE, "fixHdr: cmd %s - len %d - cur %d\n", _mqttProtCmd2Str(pMssg->e_Cmd), pMssg->u32_TotalLen, pMssg->u32_Cur));    
    }
    else
    {
        return E_MQTT_INT_CODE_BUF_END;
    }
    
    return E_MQTT_INT_CODE_OK;

}
/*!
    \brief parse payload for a connect control packet
    \param[in/out]      pMssg   pointer to message object
    \return             E_MQTT_INT_CODE     either successful or error code.

    Protocolname | m | [0][1]         u16 len
                       [2]..[2+len-1] name : "MQTT", 
                       [2+len]        protocol version: must '4'
    Connect flag  | m | [0]            bit coded (user,pwd,will retain, willQos(2),will, clean,reserved )
    Keep alive   | m | [0][1]         u16 alive intervall (sec)
    
    payload appears in the order Client Identifier, Will Topic, Will Message, User Name, Password
    
*/
E_MQTT_INT_CODE         mqttCmdConnectParse(PST_MQTT_MSSG pMssg)
{
    // variable header:
    u16 u16_Len = D_MQTT_U16_Value_GET(pMssg->pu8_Buf,pMssg->u32_Cur);
    //((u16)pMssg->pu8_Buf[pMssg->u32_Cur])<<8 | pMssg->pu8_Buf[pMssg->u32_Cur+1];

    //printf("protSize %02x %02x  %d\n",pMssg->pu8_Buf[pMssg->u32_Cur],pMssg->pu8_Buf[pMssg->u32_Cur+1], u16_Len);
    
    pMssg->u32_Cur +=2;
    // protName 
    pMssg->u32_Cur += u16_Len;
    // prot level
    printf("PROT: %d\n", pMssg->pu8_Buf[pMssg->u32_Cur]);
    pMssg->u32_Cur ++;
    // connect flags
    pMssg->u32_Cur ++;
    if ( pMssg->u32_Cur+2 <= pMssg->u32_TotalLen)
    {
        pMssg->u_PaylParameter.st_Connect.u32_KeepAlive =  D_MQTT_U16_Value_GET(pMssg->pu8_Buf,pMssg->u32_Cur);
        
        pMssg->u32_Cur +=2;
    }
    // payload: client ID 
    if( pMssg->u32_Cur < pMssg->u32_TotalLen)
    {
        // client-ID
        u16 u16_ClientLen =  D_MQTT_U16_Value_GET(pMssg->pu8_Buf,pMssg->u32_Cur);
        
        pMssg->u_PaylParameter.st_Connect.pu8_ClientId = pMssg->pu8_Buf+ pMssg->u32_Cur;

        pMssg->u32_Cur += 2 + u16_ClientLen;

    }
    // user/pwd/retain/lastwill
    pMssg->u32_Cur = pMssg->u32_BufLen;

    return E_MQTT_INT_CODE_OK;
}

int         mqttConnectAckTransmit(int h_Connection, PST_MQTT_MSSG_CONACK pst_Mssg)
{
    u8  u8_Buf[4]={0x20,0x02,0x00,0x00};

    u8_Buf[3] = pst_Mssg->e_RetCode;
    u8_Buf[1] |= pst_Mssg->u8_SessionFlag;

    return vmsrvBufferTransmit(h_Connection, u8_Buf, sizeof(u8_Buf));
}

E_MQTT_INT_CODE         mqttCmdPingParse(PST_MQTT_MSSG pMssg)
{
    UNUSED(pMssg);
    return E_MQTT_INT_CODE_OK;    
}

int         mqttPingResponseTransmit(int h_Connection)
{
    u8  u8_Buf[2] = {0xD0,0x00};
    
    return vmsrvBufferTransmit(h_Connection, u8_Buf, sizeof(u8_Buf));
}

E_MQTT_INT_CODE         mqttCmdSubscribeParse(PST_MQTT_MSSG pMssg)
{
    if( pMssg->u32_Cur < pMssg->u32_BufLen)
    {
        //printf( "%02x %02x %02x %02x\n", pMssg->pu8_Buf[pMssg->u32_Cur],pMssg->pu8_Buf[pMssg->u32_Cur+1],pMssg->pu8_Buf[pMssg->u32_Cur+2],pMssg->pu8_Buf[pMssg->u32_Cur+3]);
        // packet ID
        pMssg->u16_MssgId = ((u16)pMssg->pu8_Buf[pMssg->u32_Cur])<<8 | pMssg->pu8_Buf[pMssg->u32_Cur+1];
        pMssg->u32_Cur +=2;
        // topic ID
        pMssg->u_PaylParameter.st_Subscribe.pu8_Topic = pMssg->pu8_Buf + pMssg->u32_Cur;
        pMssg->u16_TopicLen = ((u16)pMssg->pu8_Buf[pMssg->u32_Cur])<<8 | pMssg->pu8_Buf[pMssg->u32_Cur+1];
        pMssg->u32_Cur +=2;
        pMssg->psz_Topic = (PCHAR)(pMssg->pu8_Buf+pMssg->u32_Cur);
        pMssg->u32_Cur += pMssg->u16_TopicLen;
        // QoS data, later needed \todo needs to be used if QoS > 0
        // u8_DeliveryQos = pMssg->pu8_Buf[pMssg->u32_Cur];
        pMssg->u32_Cur ++;
    }
    else
    {
        //printf("subscribe nor more topics %d/%d", pMssg->u32_Cur, pMssg->u32_BufLen);

        return E_MQTT_INT_CODE_BUF_END;
    }

    return E_MQTT_INT_CODE_OK;    
}

int         mqttSubscribeAckTransmit(int h_Connection, u16 u16_MssgId, u8 u8_Qos)
{
    u8 u8_Buf[]={0x90,0x03,0x00,0x01,0x00};
    u8_Buf[2] = (u8)((u16_MssgId>>8)&0xFF);
    u8_Buf[3] = (u8) (u16_MssgId&0xFF);
    u8_Buf[4] = u8_Qos;

    return vmsrvBufferTransmit(h_Connection, u8_Buf, sizeof(u8_Buf));
}

/*!
    \brief      parse a mqtt publish message

    [0],[1]                      topic_len
    [2]..[x= 1+topic_len]        topic
    [x],[x+1]                    packet ID
    [x+2]..[total_len-fixHdr_len] payload of given mssg
*/

E_MQTT_INT_CODE         mqttCmdPublishParse(PST_MQTT_MSSG pMssg)
{
    //topic len u16
    pMssg->u_PaylParameter.st_Publish.pu8_Topic = pMssg->pu8_Buf + pMssg->u32_Cur;
    pMssg->u16_TopicLen = ((u16)pMssg->pu8_Buf[pMssg->u32_Cur])<<8 | pMssg->pu8_Buf[pMssg->u32_Cur+1];
    pMssg->u32_Cur +=2;
  
    pMssg->psz_Topic    = (PCHAR)(pMssg->pu8_Buf + pMssg->u32_Cur);
    pMssg->u32_Cur += pMssg->u16_TopicLen;
 
    if ( D_MQTT_FLAGS_QOS_GET(pMssg->u8_Flags) != 0)
    {
        pMssg->u16_MssgId   =  ((u16)pMssg->pu8_Buf[pMssg->u32_Cur])<<8 | pMssg->pu8_Buf[pMssg->u32_Cur+1];
        pMssg->u32_Cur +=2;
    }

    pMssg->pu8_PaylStart = pMssg->pu8_Buf + pMssg->u32_Cur;
    pMssg->u16_PaylLen   = (pMssg->u32_TotalLen+2) - pMssg->u32_Cur;

    pMssg->u32_Cur += pMssg->u16_PaylLen;

    return E_MQTT_INT_CODE_OK;
}

//*/