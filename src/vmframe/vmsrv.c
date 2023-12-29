#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "platform.h"
#include "vmdebug.h"
#include "vmnet.h"

#define PORT 1883
#define SIZE 1024
#define D_VMSRV_CLIENT_MAX      5

#define         D_MOD_VMNET     "vmnet_srv"
typedef enum
{
    E_VMSRV_CON_STATUS_CLOSED,
    E_VMSRV_CON_STATUS_OPEN,
    E_VMSRV_CON_STATUS_ACTIVE,
    E_VMSRV_CON_STATUS_REL,
    E_VMSRV_CON_STATUS_MAX
} E_VMSRV_CON_STATUS;

typedef struct  _st_con_ctrl
{
    PVOID               pv_Ref;
    PFN_NOTIFY          pfn_Notify;
    int                 h_Client;
    pthread_t           pthread_Id;
    E_VMSRV_CON_STATUS  e_Status;
}ST_CON_CTRL, * PST_CON_CTRL;

ST_CON_CTRL     gst_ConCtrl[D_VMSRV_CLIENT_MAX]={0};

static   PST_CON_CTRL       _vmsrvConnectionNew(void)
{
    int i;
    for(i=0; i < D_VMSRV_CLIENT_MAX; i++)
    {
        if ( gst_ConCtrl[i].e_Status == E_VMSRV_CON_STATUS_CLOSED)
        {
            gst_ConCtrl[i].e_Status = E_VMSRV_CON_STATUS_OPEN;
            VM_PRINT_DBG((D_MOD_VMNET, E_VMDEBUG_VERBOSE, "new client connection on slot %d\n", i));
            
            return gst_ConCtrl + i;
        }
    }

    VM_PRINT_DBG((D_MOD_VMNET, E_VMDEBUG_ERR, "new client could not be allocated!\n"));
    return VM_NULL;
}

static  void                _vmsrvConnectionDel(int h_Client)
{
    int i;
    for(i=0; i < D_VMSRV_CLIENT_MAX; i++)
    {
        if ( gst_ConCtrl[i].h_Client == h_Client)
        {
            VM_PRINT_DBG((D_MOD_VMNET, E_VMDEBUG_VERBOSE, "client connection on slot %d deleted\n", i));
          
            memset((u8*)(gst_ConCtrl+i),0,sizeof(ST_CON_CTRL));
            break;
        }
    }
}

int creat_socket(PCHAR psz_IP, u16 u16_Port)
{
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    //int bindResult = bind(server_socket, (struct sockaddr *)&addr, sizeof(addr));
    //int listenResult = listen(server_socket, 5);
    bind(server_socket, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_socket, 5);

    return server_socket;
}

int wait_client(int server_socket)
{
    struct sockaddr_in cliaddr;
    int addrlen = sizeof(cliaddr);

    VM_PRINT_LOG((D_MOD_VMNET, E_VMDEBUG_INFO, "waiting for new client connection\n"));
 
    int client_socket = accept(server_socket, (struct sockaddr *)&cliaddr, &addrlen);
    VM_PRINT_LOG((D_MOD_VMNET, E_VMDEBUG_INFO, "accept new client connection from %s\n",inet_ntoa(cliaddr.sin_addr)));
   
    return client_socket;
}

void *socket_handler(void *socket_desc)
{
    PST_CON_CTRL    pst_Connection = (PST_CON_CTRL)socket_desc;
 
    int client_socket = pst_Connection->h_Client;
    int i;

    char buf[SIZE];

    VM_PRINT_DBG((D_MOD_VMNET, E_VMDEBUG_INFO, "client slot %d handle=%x started\n", pst_Connection-gst_ConCtrl, pst_Connection->h_Client));

    pst_Connection->e_Status = E_VMSRV_CON_STATUS_ACTIVE;

    while (1)
    {
        int bufSize = read(client_socket, buf, SIZE - 1);
    
        if (bufSize == 0)
        {
            if ( pst_Connection->pfn_Notify)
            {   
                pst_Connection->pfn_Notify(pst_Connection->pv_Ref, pst_Connection->h_Client, VMNET_EV_NET_ERROR, (PVOID)buf, (u32)bufSize);
            }
            break;
        }
        if (bufSize == -1)
        {
            break;
        }
        
        buf[bufSize] = '\0';
        /*printf("[vmsrv] INFO ");
        for(i=0;i< bufSize; i++)
        {
            printf("%02x ", (u8)buf[i]);
            if((i&0xF) == 0 && i != 0)
            {
                printf("\n[vmsrv] INFO ");
            }
        }

        printf("\n");
        */

        if ( pst_Connection->pfn_Notify)
        {   
            if( pst_Connection->pfn_Notify(pst_Connection->pv_Ref, pst_Connection->h_Client, VMNET_EV_PAYL_RECEIVE, (PVOID)buf, (u32)bufSize) == D_VMNET_SOCKET_CLOSING)
            {
                break;
            }
        }
    }

    VM_PRINT_DBG((D_MOD_VMNET, E_VMDEBUG_INFO, "[vmsrv] INFO socket handler on slot %d handle=%x stopped!\n",pst_Connection-gst_ConCtrl, pst_Connection->h_Client));
   
    pst_Connection->e_Status = E_VMSRV_CON_STATUS_REL;

    close(pst_Connection->h_Client);

    _vmsrvConnectionDel(pst_Connection->h_Client);

    return 0;
}

int     vmsrvBufferTransmit(int h_Handle, u8* pu8_Buf, u16 u16_Size)
{
    return (int)write(h_Handle, pu8_Buf, u16_Size);
}

int     vmsrvStart(PVOID pv_Ref, PCHAR psz_IP, u16 u16_Port, PFN_NOTIFY pfn_Notify)
{
    int nSrvHandle = -1;

    VM_PRINT_LOG(( D_MOD_VMNET, E_VMDEBUG_INFO, "[vmsrv] LOG srv started on %s:%d - %lx\n", psz_IP, u16_Port, (u32)pv_Ref));

    nSrvHandle = creat_socket(psz_IP, u16_Port);

    if ( nSrvHandle >= 0)
    {
        while(1)
        {
            PST_CON_CTRL pst_Connection = _vmsrvConnectionNew();
            if (pst_Connection)
            {
                pst_Connection->h_Client     = wait_client(nSrvHandle);
                pst_Connection->pfn_Notify   = pfn_Notify;
                pst_Connection->pv_Ref       = pv_Ref;
                printf("_h_cli: %x\n", pst_Connection->h_Client);
                if (pst_Connection->h_Client)
                {
                    pthread_t id;
            
                    pthread_create(&pst_Connection->pthread_Id, NULL, (void *)socket_handler, (void *)pst_Connection);
            
                    pthread_detach(pst_Connection->pthread_Id);
                    
                }
            }
            //else not enough connection object available
        }  
    }

    return 0;
}