#ifndef _AHT_SIM800_H__
#define _AHT_SIM800_H__

#include <Arduino.h>
#ifndef ESP32
#include <SoftwareSerial.h>
#endif

#include "ultils.h"
#include "aht_gsm.h"

enum CALL_STATUS
{
    DIALING,
    ACTIVE,
    ALERTING, // ring
    DISCONNECT
};

class AHT_SIM800: public AHT_GSM 
{
    public:
        AHT_SIM800(): AHT_GSM() {};
        AHT_SIM800(HardwareSerial *uart): AHT_GSM(uart) {};
        #ifndef ESP32
        AHT_SIM800(SoftwareSerial *uart): AHT_GSM(uart) {};
        #endif

        AHT_SIM800(const AHT_GSM *gsm);
        
        void    hello();

        // IMEI
        bool    getIMEI         (char* imei);
        bool    getSimIMEI      (char* simImei);
        
        // GPS
        bool    initLocation    ();
        bool    getLocation     (char* lat, char* lng);
        bool    getCellId       (char* cellId, char* lac);

        // SMS
        char    numSMS          (byte status);
        char    readSMS         (uint8_t index, char* phone, char* msg);
        char    deleteAllSMS    ();
        char    deleteSMS       (uint8_t index);
        char    sendSMS         (const char* phone, const char* content);
        
        void    printPhone      (const char* phone);
        void    printSMS        (const char* sms);
        void    printlnSMS      (const char* sms);

        // CALL
        bool    setupCall       ();
        char    handup          ();
        char    phoneActiveSTT  ();
        char    call            (const char* phone);
        uint16_t call           (const char* phone, uint16_t timeWait, uint16_t timeCall);

        // TIME NTP 
        bool    setupNTP        (const char* ntpServer = "pool.ntp.org", const byte timezone = 7);
        bool    getTimeNTP      (uint16_t* year, uint16_t* month, uint16_t* day, uint16_t* hour,
                                uint16_t* minute, uint16_t* second, uint16_t* timezone);

        // NETWORK
        char    attackGPRS      (const char* domain, const char* uname, const char* pword);
        char    dettachGPRS     ();
        char    connectTCP      (const char* server, uint16_t port);
        char    disconnectTCP   ();
        char    startSendTCP    ();
        char    inMultiConnection ();
        char    requestGet      (const char* server, const char* path, int port, char* body, int len);
        char    requestPost     (const char* server, const char* path, int port, const char* data, char* body, int len);
        
};

#endif
