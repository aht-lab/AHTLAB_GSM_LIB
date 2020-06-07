#ifndef _AHT_SIM800_CLIENT_H__
#define _AHT_SIM800_CLIENT_H__ 

#include "Arduino.h"
#include "Client.h"
#include "IPAddress.h"
#include "Stream.h"

#include "aht_gsm.h"

#define STATE_TCPCONNECTED 1

class AHT_GSM_Client: public Client {

    public:
        AHT_GSM_Client() {}
        AHT_GSM_Client(AHT_GSM* gsm) : modem(gsm) {}

        void        setModem    (AHT_GSM*);

        int         connect     (IPAddress ip, uint16_t port);
        int         connect     (const char *host, uint16_t port);
        size_t      write       (uint8_t);
        size_t      write       (const uint8_t *buf, size_t size);
        int         available   ();
        int         read        ();
        int         read        (uint8_t *buf, size_t size);
        int         peek        ();
        void        flush       ();
        void        stop        ();
        uint8_t     connected   ();

        operator bool() { return connected(); }

    private:
        AHT_GSM*    modem;
        byte     state = 0;
};


#endif