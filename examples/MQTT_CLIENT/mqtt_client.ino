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

#include <Client.h>
#include <aht_gsm_client.h>
Client *netClient;
bool gprs = false;

#include <MQTT.h>
MQTTClient client;
unsigned long lastMillis = 0;

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
        netClient = new AHT_GSM_Client(gsm);
        gprs = gsm->attackGPRS("internet.wind", "", "");
        
        client.begin("postman.cloudmqtt.com", 17881, *netClient);
        client.onMessage(messageReceived);

        connect();
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

    client.loop();
    delay(10);  // <- fixes some issues with WiFi stability

    if (!client.connected()) 
    {
        connect();
    }

    // publish a message roughly every second.
    if (millis() - lastMillis > 1000) 
    {
        lastMillis = millis();
        client.publish("/hello", "world");
    }
}

void connect() {
    Serial.print("checking wifi...");
    while (!gprs) 
    {
        gprs = gsm->attackGPRS("internet.wind", "", "");
    }

    Serial.print("\nconnecting...");
    while (!client.connect("esp32-gsm", "zxyndpfc", "X2fbxwwciAbu")) 
    {
        Serial.print(".");
        delay(1000);
    }

    Serial.println("\nconnected!");

    client.subscribe("/hello");
}

void messageReceived(String &topic, String &payload) {
    Serial.println("incoming: " + topic + " - " + payload);
}
