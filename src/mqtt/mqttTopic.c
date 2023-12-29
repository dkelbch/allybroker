/*!
    TOPIC management
        use a list structure on a buffer array.
        freeList and each client has a used list

        #                                       : all information
        dev/#                                   : all dev information
        dev/{type}/#                            : all {dev type} information
        dev/{type}/{id}/#                       : all {dev type} on specific device information


        dev/motion/#
        dev/motion/+/action
        dev/motion/0123456789asdf/status
        dev/motion/0123456789asdf/cmd
        dev/motion/0123456789asdf/action

*/

#include    "platform.h"
#include    "linklist.h"
#include    "mqtt.h"

#define         D_MOD_TOPIC     "topicHdl"

#define         D_MQTT_TOPIC_WILDCAT_HASH           1
#define         D_MQTT_TOPIC_WILDCAT_PLUS           2


typedef struct _topicItem
{
    LINK            Link;
    PST_MQTT_CLIENT pSubClients[D_MQTT_CLIENT_NUM];
    u8              u8_Topic[D_MQTT_TOPIC_SIZE];
    u8              u8_LevelDepth;
    u8              u8_Wildcat;
}ST_TOPIC_ITEM, * PST_TOPIC_ITEM;

// pool of usable topic item 
ST_TOPIC_ITEM       gst_TopicItem[D_MQTT_TOPIC_NUM] ={0};
LIST                gst_TopicFreelist               ={0};
// list of being subscribed topics on that broker
LIST                gst_TopicSubscription           ={0};

static void         _topicClientAttach(PST_TOPIC_ITEM pItem, PST_MQTT_CLIENT pst_Client)
{
    int i;
    for(i=0;i< D_MQTT_CLIENT_NUM; i++)
    {
        if(pItem->pSubClients[i] == VM_NULL)
        {
            VM_PRINT_DBG((D_MOD_TOPIC, E_VMDEBUG_INFO, "add client hdl %d on slot %d - %s\n"
                            , pst_Client->h_Connection, i, (PCHAR)pItem->u8_Topic));
            
            pItem->pSubClients[i] = pst_Client;
            break;
        }
    }
}

/*
static void         _topicClientDettach(PST_TOPIC_ITEM pItem, PST_MQTT_CLIENT pst_Client)
{
    int i;
    for(i=0;i< D_MQTT_CLIENT_NUM; i++)
    {
        if(pItem->pSubClients[i] == pst_Client)
        {
            pItem->pSubClients[i] = VM_NULL;
            break;
        }
    }
}
*/

void                _topicItemFree(PST_TOPIC_ITEM pst_Item)
{
    memset((u8*)pst_Item, 0, sizeof(ST_TOPIC_ITEM));
    llAttachTail(&gst_TopicFreelist, (PLINK)pst_Item);
}


static  void        _topicItemSusbcriptListAdd(PST_TOPIC_ITEM pst_Item)
{
    //\todo need a sorted list
    llAttachHead(&gst_TopicSubscription, (PLINK)pst_Item);
}

static  void        _topicItemAnalyze(PST_TOPIC_ITEM pItem)
{
    unsigned int     i;
    unsigned int     tlen = strlen((PCHAR)(pItem->u8_Topic));
    pItem->u8_LevelDepth = 0;

    for(i=0;i<tlen; i++)
    {
        //printf("[%c] ", pItem->u8_Topic);
        if(pItem->u8_Topic[i]=='/')
        {
            //printf("zterm\n");
            pItem->u8_LevelDepth++;
            pItem->u8_Topic[i] = 0;         // separate topic level to use string function
        }
        else if( pItem->u8_Topic[i]=='#') 
        {
            //printf("wildHash\n");
            pItem->u8_Wildcat |= D_MQTT_TOPIC_WILDCAT_HASH;
        }
        else if (pItem->u8_Topic[i]=='+')
        {
            //printf("wildPLUS\n");
            pItem->u8_Wildcat |= D_MQTT_TOPIC_WILDCAT_PLUS;
        }
        else
        {
            //printf("\n");
            // check if character is valid.....
        }
    }
}

static  int         _topicCompare(u8* psz_Orig, u8 u8_LevelO, u8* psz_Dest, u8 u8_LevelD)
{
    unsigned int i=0;
    PCHAR psz_O, psz_D;

    psz_O = (PCHAR)psz_Orig;
    psz_D = (PCHAR)psz_Dest;

    if ( u8_LevelO > u8_LevelD)
    {
        return VM_FALSE;
    }
    else 
    {
        for(i=0; i < (u8_LevelO+1);i++)
        {
            VM_PRINT_DBG((D_MOD_TOPIC, E_VMDEBUG_INFO, "lev %d %s ?? %s\n", i,psz_O, psz_D));
    
            if( strlen(psz_O) != strlen(psz_D))
            {
                // check if wildcat or not
                if ( psz_O[0] == '#')
                {
                    return VM_TRUE;
                }

                if( psz_O[0] != '+')
                {
                    return VM_FALSE;
                }
            }
            else
            {
                if(strncmp(psz_O, psz_D, strlen(psz_O)))
                {
                    return VM_FALSE;
                }
            }

            psz_O += strlen(psz_O)+1;
            psz_D += strlen(psz_D)+1;
        }
    }

    return VM_TRUE;

}

void                topicItemListInitialize(void)
{
    unsigned int i;
    for(i=0; i< sizeof(gst_TopicItem)/sizeof(ST_TOPIC_ITEM); i++)
    {
        llAttachTail(&gst_TopicFreelist,(PLINK)(gst_TopicItem + i));
    }
    VM_PRINT_DBG((D_MOD_TOPIC, E_VMDEBUG_INFO, "initialized topic handler\n"));
}


PST_TOPIC_ITEM      topicItemFindByTopic(PLIST pList, PCHAR psz_Topic, PST_TOPIC_ITEM pst_Cur)
{
    PST_TOPIC_ITEM  pst_Item     = VM_NULL;
    ST_TOPIC_ITEM   st_TmpItem   = {0};

    VM_PRINT_DBG((D_MOD_TOPIC, E_VMDEBUG_INFO, "find the topic %s\n", psz_Topic));

    strncpy( (PCHAR)st_TmpItem.u8_Topic, psz_Topic, strlen(psz_Topic));
    
    _topicItemAnalyze(&st_TmpItem);

    if ( pst_Cur != VM_NULL)
    {
        pst_Item = (PST_TOPIC_ITEM)NEXT((PLINK)(&pst_Cur->Link));
        if ( pst_Item)
        {
            VM_PRINT_DBG((D_MOD_TOPIC, E_VMDEBUG_VERBOSE, "subList next %s\n", pst_Item->u8_Topic));
        }
    }
    else
    {
        pst_Item = (PST_TOPIC_ITEM)HEAD(pList);
        if ( pst_Item)
        {
            VM_PRINT_DBG((D_MOD_TOPIC, E_VMDEBUG_VERBOSE, "subList head %s\n", pst_Item->u8_Topic));
        }
    }

    while( pst_Item != VM_NULL)
    {
        VM_PRINT_DBG((D_MOD_TOPIC, E_VMDEBUG_VERBOSE, "cur %s, levs %d\n", pst_Item->u8_Topic, pst_Item->u8_LevelDepth));

        if (pst_Item->u8_LevelDepth == st_TmpItem.u8_LevelDepth)
        {
            //similar topic string
            if ( _topicCompare(pst_Item->u8_Topic, pst_Item->u8_LevelDepth, st_TmpItem.u8_Topic, st_TmpItem.u8_LevelDepth) )
            {
                return pst_Item;
            }
        }
        else if (pst_Item->u8_LevelDepth < st_TmpItem.u8_LevelDepth)
        {
            // check for '#'
            VM_PRINT_DBG(( D_MOD_TOPIC, E_VMDEBUG_VERBOSE, "compare with wildcat %02x\n", pst_Item->u8_Wildcat));

            if((pst_Item->u8_Wildcat& D_MQTT_TOPIC_WILDCAT_HASH) == D_MQTT_TOPIC_WILDCAT_HASH)
            {
               if ( _topicCompare(pst_Item->u8_Topic, pst_Item->u8_LevelDepth, st_TmpItem.u8_Topic, st_TmpItem.u8_LevelDepth) )
                {
                    return pst_Item;
                } 
                else
                {
                    VM_PRINT_DBG(( D_MOD_TOPIC, E_VMDEBUG_VERBOSE, "does not match with wildcat\n" ));
                }
            }
        }
        else
        {
            // do nothing
        }
        
        pst_Item = (PST_TOPIC_ITEM)NEXT(&pst_Item->Link);
        if ( pst_Item )
        {
            VM_PRINT_DBG(( D_MOD_TOPIC, E_VMDEBUG_VERBOSE, "--------------------------------\n" ));
        }
    };

    return VM_FALSE;
}

E_MQTT_INT_CODE      topic2subcriptionListAdd(PCHAR psz_Topic, PST_MQTT_CLIENT pst_Client )
{
    E_MQTT_INT_CODE e_Ret = E_MQTT_INT_CODE_OK;

    PST_TOPIC_ITEM pst_Item = (PST_TOPIC_ITEM)llDetachHead(&gst_TopicFreelist);

    if (pst_Item != VM_NULL)
    {
        PST_TOPIC_ITEM  pst_TmpItem = VM_NULL;

        VM_PRINT_LOG((D_MOD_TOPIC, E_VMDEBUG_INFO, "add topic %s to subList\n", psz_Topic));
        // set-tup item here.
        memset((u8*)pst_Item, 0, sizeof(ST_TOPIC_ITEM));
        strcpy( (PCHAR)pst_Item->u8_Topic, psz_Topic);
        _topicClientAttach( pst_Item, pst_Client);
        
        _topicItemAnalyze(pst_Item);

        pst_TmpItem = (PST_TOPIC_ITEM)HEAD(&gst_TopicSubscription);
        while(pst_TmpItem)
        {
            if( !memcmp(pst_TmpItem->u8_Topic, pst_Item->u8_Topic, sizeof(pst_Item->u8_Topic) ) )
            {
                break;
            }
            pst_TmpItem = (PST_TOPIC_ITEM)NEXT((PLINK)pst_TmpItem);
        }
         
        if ( pst_TmpItem == VM_NULL)
        {
            _topicItemSusbcriptListAdd(pst_Item);
        }
        else
        {
            VM_PRINT_DBG((D_MOD_TOPIC, E_VMDEBUG_INFO, "topic already exist, add client to that topic\n"));

            _topicItemFree(pst_Item);
        }
    }
    else
    {
        VM_PRINT_LOG((D_MOD_TOPIC, E_VMDEBUG_ERR, "topicItem freeList empty, can't handle topic %s\n", psz_Topic));

        e_Ret = E_MQTT_INT_CODE_NO_MEM;
    }

    return e_Ret;
}

void                top2subscriptionListClientRemove(PST_MQTT_CLIENT pst_Client)
{
    int i,nFree=0;

    PST_TOPIC_ITEM  pCurItem     = VM_NULL;
    PST_TOPIC_ITEM  pTmpItem     = VM_NULL;
   
    VM_PRINT_LOG((D_MOD_TOPIC, E_VMDEBUG_INFO, "remove client hdl %d from all topics\n", pst_Client->h_Connection));
   
    pCurItem = (PST_TOPIC_ITEM)HEAD(&gst_TopicSubscription);
    while(pCurItem)
    {
        pTmpItem = (PST_TOPIC_ITEM)NEXT((PLINK)pCurItem);
        
        for (i=0; i < D_MQTT_CLIENT_NUM; i++)
        {
            if ( pCurItem->pSubClients[i] == pst_Client)
            {          
                pCurItem->pSubClients[i] = VM_NULL; 
                nFree++;  
            }
            else if(pCurItem->pSubClients[i] ==VM_NULL)
            {
                nFree++;
            }
            //else{
            //    printf("[%d]occupied %d\n",i, pCurItem->pSubClients[i]->h_Connection);
            //}
        }
       
        if( nFree >= D_MQTT_CLIENT_NUM )
        {
            VM_PRINT_DBG((D_MOD_TOPIC, E_VMDEBUG_INFO, "remove topic from subList\n" ));

            llDetachLink(&gst_TopicSubscription, (PLINK)pCurItem);
            _topicItemFree(pCurItem);
        }

        pCurItem = pTmpItem;
        nFree=0;
    }
}

E_MQTT_INT_CODE     topicPublishForwarder(PST_MQTT_MSSG pMssg)
{
    E_MQTT_INT_CODE e_Ret                       = E_MQTT_INT_CODE_OK;
    PST_TOPIC_ITEM  pItem                       = VM_NULL;
    u8              sz_Topic[D_MQTT_TOPIC_SIZE] = {0};
    int             i,bo_ClientUsed               = VM_FALSE;
    
    memcpy(sz_Topic, pMssg->psz_Topic, MIN(D_MQTT_TOPIC_SIZE-1, pMssg->u16_TopicLen));

    pItem = topicItemFindByTopic(&gst_TopicSubscription, (PCHAR)sz_Topic, VM_NULL);

    if(pItem)
    {
        while(pItem)
        {
            for(i=0;i < D_MQTT_CLIENT_NUM; i++)
            {
                if( pItem->pSubClients[i])
                {
                    mqttTractMssgForward(pItem->pSubClients[i]->h_Connection, pMssg->pu8_Buf, pMssg->u32_TotalLen+2 );
                    bo_ClientUsed = VM_TRUE;
                }
            }
            
            if ( bo_ClientUsed == VM_FALSE)
            {
                VM_PRINT_DBG(( D_MOD_TOPIC, E_VMDEBUG_WARN, "no client is bind to topic\n"));

                e_Ret = E_MQTT_INT_CODE_NOT_FIND;
            }

            VM_PRINT_DBG((D_MOD_TOPIC, E_VMDEBUG_VERBOSE, "-----------  FIND NEXT ITEM Topic  -----------\n"));

            pItem = topicItemFindByTopic(&gst_TopicSubscription, (PCHAR)sz_Topic, pItem);            
        }    
    }
    else
    {    
        VM_PRINT_LOG((D_MOD_TOPIC, E_VMDEBUG_WARN, "That topic is not in subList\n"));
        e_Ret = E_MQTT_INT_CODE_NOT_FIND;
    }

    return e_Ret;
}

u32                topicStatisticGet(void)
{
    u32            u32_Cnt  =   0;
    PST_TOPIC_ITEM pItem    =   (PST_TOPIC_ITEM)HEAD(&gst_TopicSubscription);
    
    while(pItem)
    {
        printf("[topic] topic - %s\n", (PCHAR)pItem->u8_Topic);
        u32_Cnt++;
        pItem = (PST_TOPIC_ITEM)NEXT((PLINK)pItem);
    }

    return u32_Cnt;
}
//*/