#include "platform.h"
#include "vmdebug.h"

#define     D_MOD_MAIN      "main"

typedef struct   _st_brkCtrl
{
    PCHAR   psz_Port;
    PCHAR   psz_IP;
    
} ST_BRKCTRL;

ST_BRKCTRL  gst_Broker = {
    (PCHAR)"1883",
    (PCHAR)"0.0.0.0"
};

extern int         mqttBrokerStart(PCHAR psz_IP, u16 u16_Port, PCHAR psz_Certificate);

/*!
    brief       usage of the broker
*/
void        main_usage(void)
{
    printf("usage: allyBroker -p port -ip listen IP Adress -cert certification file");

}

/*!
    brief       broker main entity
*/
int main(int argc, char** argv)
{

    // check later on argument
    VM_PRINT_DBG(( D_MOD_MAIN,E_VMDEBUG_INFO, "Broker started on %s:%s\n", gst_Broker.psz_IP, gst_Broker.psz_Port));
    mqttBrokerStart((PCHAR)gst_Broker.psz_IP, atoi(gst_Broker.psz_Port), VM_NULL);
    exit(0);
}