/*!

*/
#ifndef     MQTT_H
#define      MQTT_H

#ifndef        D_MQTT_CLIENT_NUM       // max number of used client
#define         D_MQTT_CLIENT_NUM       5
#endif // D_MQTT_CLIENT_NUM
#ifndef        D_MQTT_PAYL_SIZE         // limited payload size
#define         D_MQTT_PAYL_SIZE        256
#endif // D_MQTT_PAYL_SIZE              // max number of managed topics
#ifndef        D_MQTT_TOPIC_NUM
#define         D_MQTT_TOPIC_NUM        256
#endif // D_MQTT_TOPIC_NUM  
#ifndef        D_MQTT_TOPIC_SIZE        // max length of topic
#define         D_MQTT_TOPIC_SIZE       128
#endif // D_MQTT_TOPIC_SIZE             // max length of client ID
#ifndef        D_MQTT_CLIENT_ID_SIZE   
#define         D_MQTT_CLIENT_ID_SIZE   128
#endif // D_MQTT_CLIENT_ID_SIZE    

typedef     enum
{
    E_MQTT_INT_CODE_OK,
    E_MQTT_INT_CODE_BUF_END,
    E_MQTT_INT_CODE_ERR_PARSE,
    E_MQTT_INT_CODE_NO_MEM,
    E_MQTT_INT_CODE_CLOSE,
    E_MQTT_INT_CODE_NOT_FIND,
    E_MQTT_INT_CODE_MAX
}E_MQTT_INT_CODE;

typedef enum
{
    E_MQTT_CMD_UNDEF,
    E_MQTT_CMD_CONNECT,
    E_MQTT_CMD_CONNECT_ACK,
    E_MQTT_CMD_PUBLISH,
    E_MQTT_CMD_PUBLISH_ACK,
    E_MQTT_CMD_PUBLISH_REC,
    E_MQTT_CMD_PUBLISH_REL,
    E_MQTT_CMD_PUBLICH_CPLT,
    E_MQTT_CMD_SUBSCRIBE,
    E_MQTT_CMD_SUBSCRIBE_ACK,
    E_MQTT_CMD_UNSUBSCRIBE,
    E_MQTT_CMD_UNSUBSCRIBE_ACK,
    E_MQTT_CMD_PING_REQ,
    E_MQTT_CMD_PING_RSP,
    E_MQTT_CMD_DISCONNECT
} E_MQTT_CMD;

#define     D_MQTT_CMD_GET(x)       (((x)&0xF0)>>4)
#define     D_MQTT_CMD_SET(x)       ((x)<<4)

#define     D_MQTT_FLAGS_GET(x)     ((x)&0x0F)
#define     D_MQTT_FLAGS_QOS_GET(x) (((x)&0x06)>>1)
//fkags do not care...
#define     D_MQTT_CONNECT_FLAGS_USR_GET(x)     ((x)&0x80)
#define     D_MQTT_CONNECT_FLAGS_USR_SET(x)     ((x)|0x80)
#define     D_MQTT_CONNECT_FLAGS_PWD_GET(x)     ((x)&0x40)
#define     D_MQTT_CONNECT_FLAGS_PWD_SET(x)     ((x)|0x40)
//rest bits not used,here

#define     D_MQTT_U16_Value_GET(p,idx) ((u16)(p)[(idx)])<<8 | (p)[(idx)+1];

#define     D_MQTT_STRING_LEN(p) ((u16)(p)[0])<<8 | (p)[1]
#define     D_MQTT_STRING_VALUE(p) (((PCHAR)(p))+2)

typedef enum 
{
    E_MQTT_CONACK_RET_CODE_ACCPT,
    E_MQTT_CONACK_RET_CODE_PROT_ERR,
    E_MQTT_CONACK_RET_CODE_ID_ERR,
    E_MQTT_CONACK_RET_CODE_SRV_UNAVAIL,
    E_MQTT_CONACK_RET_CODE_AUTH_BAD,
    E_MQTT_CONACK_RET_CODE_AUTH_ERR,
    E_MQTT_CONACK_RET_CODE_MAX
}E_MQTT_CONACK_RET_CODE;

typedef struct _st_mqtt_mssg_conack
{
    E_MQTT_CONACK_RET_CODE  e_RetCode;
    u8                      u8_SessionFlag;
}ST_MQTT_MSSG_CONACK, * PST_MQTT_MSSG_CONACK;

typedef struct _st_mqtt
{
    E_MQTT_CMD  e_Cmd;
    u8          u8_Flags;
    
    u16         u16_MssgId;
    PCHAR       psz_Topic;
    u16         u16_TopicLen;
    u8 *        pu8_PaylStart;
    u16         u16_PaylLen;
    
    //flags... later
    u32         u32_TotalLen;
    u32         u32_VarLen;
    
    u32         u32_BufLen;
    u32         u32_Cur;
    u8 *        pu8_Buf;

    //additional information
    union {
        struct 
        {
            u32  u32_KeepAlive;
            u8 * pu8_ClientId;
            u8 * pu8_WillTopic;
            u8 * pu8_Username;
            u8 * pu8_Pwd;
        } st_Connect;
        ST_MQTT_MSSG_CONACK st_ConnectAck;
        struct 
        {
            u8 * pu8_Topic;
            u8 * pu8_PacketId;
        } st_Publish;
        struct
        {
            u8 * pu8_PacketId;
        } st_PublishAck;
        struct
        {
            u8 * pu8_PacketId;
            u8 * pu8_Topic; 
        } st_Subscribe;
        
    } u_PaylParameter;


}ST_MQTT_MSSG, * PST_MQTT_MSSG;

typedef struct _mqttClient
{
    int         h_Connection;
    u8          u8_ClientId[D_MQTT_CLIENT_ID_SIZE];
    //outgoing queue, if needed;
} ST_MQTT_CLIENT, * PST_MQTT_CLIENT;


#ifdef      __cpluscplus
extern "C"
{
#endif      // __cpluscplus
E_MQTT_INT_CODE         mqttFixedHeaderParse( PST_MQTT_MSSG pMssg);
E_MQTT_INT_CODE         mqttCmdConnectParse(PST_MQTT_MSSG pMssg);
E_MQTT_INT_CODE         mqttCmdSubscribeParse(PST_MQTT_MSSG pMssg);
E_MQTT_INT_CODE         mqttCmdPublishParse(PST_MQTT_MSSG pMssg);

int                     mqttConnectAckTransmit(int h_Connection, PST_MQTT_MSSG_CONACK pst_Mssg);
int                     mqttSubscribeAckTransmit(int h_Connection, u16 u16_MssgId, u8 u8_Qos);
int                     mqttPingResponseTransmit(int h_Connection);

                    E_MQTT_INT_CODE mqttTractMssgForward(int h_Connection, u8* pu8_Buf, u16 u16_BufSize);


E_MQTT_INT_CODE                     mqttTractConnect(int h_Client, PST_MQTT_MSSG pMssg);
E_MQTT_INT_CODE                     mqttTractDisconnect(int h_Client, PST_MQTT_MSSG pMssg);
E_MQTT_INT_CODE                     mqttTractSubscribe(int h_Client, PST_MQTT_MSSG pMssg);
E_MQTT_INT_CODE                     mqttTractPublish(int h_Client, PST_MQTT_MSSG pMssg);


void                    topicItemListInitialize(void);
E_MQTT_INT_CODE         topic2subcriptionListAdd(PCHAR psz_Topic, PST_MQTT_CLIENT pst_Client );
void                    top2subscriptionListClientRemove(PST_MQTT_CLIENT pst_Client);
E_MQTT_INT_CODE         topicPublishForwarder(PST_MQTT_MSSG pMssg);

u32                     topicStatisticGet(void);
#ifdef      __cpluscplus
}
#endif      // __cpluscplus
#endif      // MQTT_H

//*/