/* ======================================== */
/* SIM800 */
/* ======================================== */
#include <aht_sim800.h>
#define MODEM_RX 16
#define MODEM_TX 17
#define uart Serial2

AHT_GSM *gsmMaster = new AHT_GSM(&uart);
AHT_GSM *gsm;
GSM_TYPE gsmType;
bool gsmOk = false;

void setup() 
{
    Serial.begin(115200);
    delay(1000);

    if(gsmMaster->begin()) 
    {
        gsmType = gsmMaster->detectGSM(&uart);
        unsigned long baudrate = gsmMaster->getBaudrate();
        free(gsmMaster);
        
        if(gsmType == SIM800) 
        {
            gsm = new AHT_SIM800(&uart);
            gsm->begin(baudrate);
            gsmOk = true;
        }

        char phone[] = "0946866793";
        bool canCall = gsm->setupCall();
        if(canCall)
        {
            Serial.println("canCall");
            // call(<phone>, <thời gian chờ nghe máy>(giây), <thời gian gọi>(giây))
            // return: <thời gian 2 bên nghe máy>
            // *Lưu ý: 
            // 			<thời gian chờ nghe máy> nên phù hợp để tránh tính trạng 
            //			tổng đài yêu cầu gửi lời nhắn thoại.
            //			Trong trường hợp đó dù cuộc gọi không thành công
            //			nhưng vẫn sẽ mất thời gian gọi điện.
            uint16_t timeCall = gsm->call(phone, 35, 5);
            Serial.print("time call: ");
            Serial.println(timeCall);
        }
        
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
    }
}

void loop()
{
    if(!gsmOk)
    {
        Serial.println("can't detect gsm module");
        delay(5000);
        return;
    }

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