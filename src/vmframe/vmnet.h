#ifndef     VMNET_H
#define      VMNET_H


typedef enum
{
    VMNET_EV_UNDEF,
    VMNET_EV_INC_CLIENT,
    VMNET_EV_PAYL_RECEIVE,
    VMNET_EV_NET_ERROR,
    VMNET_EV_MAX
}VMNET_EV;
#define         D_VMNET_SOCKET_CLOSING      (-1)
#ifdef      __cpluscplus
extern "C"
{
#endif      // __cpluscplus
typedef     int (*PFN_NOTIFY) (PVOID pv_Ref, HANDLE h_Client, VMNET_EV e_Event, PVOID pv_Param, u32 u32_Params);


int         vmsrvStart( PVOID pv_Ref, PCHAR psz_IP, u16 u16_Port, PFN_NOTIFY pfn_Notify);
int         vmsrvStop();

int     vmsrvBufferTransmit(int h_Handle, u8* pu8_Buf, u16 u16_Size);
#ifdef      __cpluscplus
}
#endif      // __cpluscplus
#endif      //VMNET_H