#ifndef _AHT_GSM_H__
#define _AHT_GSM_H__

#include <Arduino.h>
#ifndef ESP32
#include <SoftwareSerial.h>
#endif

#define   _AHT_GSM_DEBUG__
#ifdef    _AHT_GSM_DEBUG__
  #define debug           Serial
  #define DB_Print(...)   debug.print(__VA_ARGS__);
  #define DB_Println(...) debug.println(__VA_ARGS__);
#else
  #define debug           Serial
  #define DB_Print(...) 
  #define DB_Println(...) 
#endif

enum GSM_TYPE 
{
    SIM5300,
    SIM800,
    UC15
};

enum UART_TYPE 
{
    HARDWARE,
    SOFTWARE
};

#define     AT_NO_RESPONSE      -1
#define     AT_TIMEOUT          -2
#define     AT_ERROR            -3
#define     AT_REPLY_NOT_FOUND  -4
#define     AT_OK               1
#define     AT_REPLY_FOUND      2
#define     AT_READ_OK          3
#define     AT_AVAILABLE        4

#define     SEG_END     1
#define     SEG_STOP    2

#define     SMS_STT_UNREAD      1
#define     SMS_STT_READ        2
#define     SMS_STT_ALL         3

#define     SMS_READ_FAIL       4
#define     SMS_READ_SUCCESS    5

#define     BUFFER_SIZE         1024

class AHT_GSM 
{
    public: 
        AHT_GSM();
        AHT_GSM(HardwareSerial* uart);
#ifndef ESP32
        AHT_GSM(SoftwareSerial* uart);
#endif
        
        virtual     ~AHT_GSM() { delete _uart; };

        // GET SET 
        uint16_t    getBaudrate();
        void        setUart (Stream* uart);
        Stream*     getUart ();
        
        // DEBUG
        void          DB_Buffer(uint16_t index);
        virtual void  hello();

        // BEGIN
        void        begin     (uint32_t baudrate);
        bool        begin     ();
        GSM_TYPE    detectGSM (const Stream* uart);
        bool        available ();

        // UART 
        void        print     (int val);
        void        print     (const char* at);
        void        println   (const char* at);
        void        println   (int val);
        
        char        readSegment         (const char* strStop, uint16_t timeout = 1000);
        char        readResponse        (const char* reply, uint8_t line, uint16_t timeout = 1000);
        char        readResponse        (uint16_t timeout = 1000);
        char        readResponse        (uint16_t timeout, const char* reply);
        char        readUntil           (uint16_t timeout, const char* reply);
        
        char        WaitForReply        (uint16_t timeout = 1000);
        char        sendAndReadResponse (const char* command, uint16_t timeout = 1000);
        char        sendAndReadResponse (const char* command, const char* reply, uint8_t line, uint16_t timeout = 1000);
        char        sendAndCheckReply   (const char* command, const char* reply, uint16_t timeout = 1000, uint8_t timeTry = 1, uint8_t line = 0);

        // IMEI
        virtual bool    getIMEI         (char* imei) {}
        virtual bool    getSimIMEI      (char* simImei) {}
        
        // GPS
        virtual bool    initLocation    () {}
        virtual bool    getLocation     (char* lat, char* lng) {}
        virtual bool    getCellId       (char* cellId, char* lac) {}

        // SMS
        virtual char    numSMS          (byte status) {}
        virtual char    readSMS         (uint8_t index, char* phone, char* msg) {}
        virtual char    deleteAllSMS    () {}
        virtual char    deleteSMS       (uint8_t index) {}
        virtual char    sendSMS         (const char* phoneNumber, const char* content) {}

        // CALL
        virtual bool    setupCall       () {}
        virtual uint16_t call           (const char* phone, uint16_t timeWait, uint16_t timeCall) {}
        virtual char    call            (const char* phone) {}
        virtual char    handup          () {}
        virtual char    phoneActiveSTT  () {}

        // TIME NTP 
        virtual bool    setupNTP        (const char* ntpServer = "pool.ntp.org", const byte timezone = 7) {};
        virtual bool    getTimeNTP      (uint16_t* year, uint16_t* month, uint16_t* day, uint16_t* hour,
                                        uint16_t* minute, uint16_t* second, uint16_t* timezone) {};

        // NETWORK
        virtual char    attackGPRS      (const char* domain, const char* uname, const char* pword) {}
        virtual char    dettachGPRS     () {}
        virtual char    connectTCP      (const char* server, uint16_t port) {}
        virtual char    disconnectTCP   () {}
        virtual char    requestGet      (const char* server, const char* path, int port, char* body, int len) {}
        virtual char    requestPost     (const char* server, const char* path, int port, const char *data, char* body, int len) {}
        virtual char    startSendTCP    () {};
        

    protected:
        Stream*     _uart;
        char        _buffer[BUFFER_SIZE]    = "";
        uint32_t    _baudrate       = 9600;
        uint8_t     _bufferLen      = 0;
        GSM_TYPE    gsmType;
        UART_TYPE   uartType;
};

#endif
