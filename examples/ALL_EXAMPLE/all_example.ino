#include "aht_sim800.h"

#define MODEM_RX 16
#define MODEM_TX 17
#define uart Serial2

AHT_GSM *gsmMaster = new AHT_GSM(&uart);
AHT_GSM *gsm;
GSM_TYPE gsmType;
bool uartOK = false;

// define
#define APN "m-wap"
#define USER "mms"
#define PASS "mms"

// Params
char LAT[10], LNG[10];
char CELL_ID[6], LAC[6];
char IMEI_GSM[20], IMEI_SIM[20];

void setup() 
{
    Serial.begin(115200);
    delay(100);
    
    uartOK = gsmMaster->begin();
    
    if(uartOK) 
    {
        gsmType = gsmMaster->detectGSM(&uart);
        unsigned long baudrate = gsmMaster->getBaudrate();
        free(gsmMaster);
        
        if(gsmType == SIM800) 
        {
            gsm = new AHT_SIM800(&uart);
            gsm->begin(baudrate);
        }
        
        
        // LOCATION, CELL ID
        if(gsm->getLocation(LAT, LNG)) 
        {
            Serial.println("Get location success");
        }
        else
        {
            Serial.println("Fail to get location");
            gsm->getCellId(CELL_ID, LAC);
        }

        // CALL
        char phone[] = "0946866793";

        // Hàm gọi 1
        bool canCall = gsm->setupCall();
        if(canCall)
        {
            Serial.println("canCall");
            // call(<phone>, <thời gian chờ bắt máy>, <thời gian gọi>)
            // return: <thời gian nghe máy>
            uint16_t timeCall = gsm->call(phone, 35, 5);
            Serial.print("time call: ");
            Serial.println(timeCall);
        }
        
        // Hàm gọi 2
        gsm->call(phone);
        unsigned long now = millis();
        unsigned long TIMEOUT = 15000;
        while(millis() - now < TIMEOUT)
        {
            int callActive = gsm->phoneActiveSTT();
            Serial.println("Call Active: " + String(callActive));
            delay(1000);
        }
        gsm->handup();

        // READ LAST SMS
        char phone[12], msg[512];
        uint8_t numSMS = gsm->numSMS(SMS_STT_ALL);
        gsm->readSMS(numSMS, phone, msg);

        Serial.print("phone: ");
        Serial.println(phone);
        Serial.print("msg: ");
        Serial.println(msg);

        // DELETE SMS
        // gsm->deleteSMS(10);

        // SEND SMS
        if(gsm->sendSMS("0946866793", "cafe5hsang - mita9497dev"))
        {
            Serial.println("Gui SMS thanh cong");
        }

        // IMEI
        gsm->getIMEI(IMEI_GSM, 20);
        Serial.print("IMEI GSM: ");
        Serial.println(IMEI_GSM);
    
        gsm->getIMEI(IMEI_SIM, 20);
        Serial.print("IMEI SIM: ");
        Serial.println(IMEI_SIM);
    }
}

void loop() 
{
    if(uart.available())
    {
        Serial.write(uart.read());
    }
    if(Serial.available())
    {
        char c = Serial.read();
        Serial.write(c);
        uart.write(c);
    }
}