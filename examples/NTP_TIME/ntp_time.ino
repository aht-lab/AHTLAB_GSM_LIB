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
bool timeInit = false;

uint16_t year, month, day, hour, minute, second, timezone;

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

            timeInit = gsm->setupNTP();
            if (timeInit)
            {
                gsm->getTimeNTP(&year, &month, &day, &hour, &minute, &second, &timezone);
                Serial.print("year: ");
                Serial.println(year);
                Serial.print("month: ");
                Serial.println(month);
                Serial.print("day: ");
                Serial.println(day);
                Serial.print("hour: ");
                Serial.println(hour);
                Serial.print("minute: ");
                Serial.println(minute);
                Serial.print("second: ");
                Serial.println(second);
                Serial.print("timezone: ");
                Serial.println(timezone);
            }
        }
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
