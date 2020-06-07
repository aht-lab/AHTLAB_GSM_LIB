/* ======================================== */
/* WIFI */
/* ======================================== */
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char WIFI_SSID[] = "AHTLAB";
const char WIFI_PASS[] = "ahtlab.com";

WiFiClient client;
IPAddress dns(8,8,8,8);

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

// INET
#define APN             "internet.wind"
#define USER            ""
#define PASS            ""
#define HTTP            "https://"
#define HOST            "5ea8480235f3720016608d00.mockapi.io"
#define PATH            "/api/test"
#define RESPONSE_LEN        128
char response[RESPONSE_LEN];

// Flags
bool gsmNet = false;

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
            Serial.println("GSM OK");
        }
        gsmNet = gsm->attackGPRS(APN, USER, PASS);
    }

    WiFi_Init();

    requestGet();
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

void requestGet()
{
    bool requestStatus = getRequest(HOST, PATH);
    if(requestStatus)
    {
        Serial.println("---RESPONSE---");
        Serial.println(response);
        Serial.println("--------------");
    }
    Serial.println();
}

bool getRequest(const char* host, const char* path)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WL_NOT_CONNECTED: GPRS Request");
        return GSM_GetRequest(host, path);
    }
    else
    {
        Serial.println("WL_CONNECTED: WIFI Request");
        return WiFi_GetRequest(host, path);
    }
}


bool GSM_GetRequest(const char* host, const char* path)
{
    if(!gsmNet)
    {
        gsmNet = gsm->attackGPRS(APN, USER, PASS);
        if(!gsmNet) return false;
    }
    
    return gsm->requestGet(host, path, 80, response, RESPONSE_LEN);
}

bool WiFi_GetRequest(const char* host, const char* path) 
{
    HTTPClient http;
    http.begin(String(HTTP) + String(host) + String(path));    
    delay(100);
    int httpResponseCode = http.GET();
    delay(1000);
    String payload = http.getString();
    Serial.print("response: ");
    Serial.println(payload);
    
    Serial.print("Http return code: ");
    Serial.println(httpResponseCode);  

    if(httpResponseCode == 200) 
    {
        Serial.println(F("wifi request success"));
        payload.toCharArray(response, RESPONSE_LEN);
        Serial.println(F("Disconnect server"));
        
        http.end(); 
        return true;
    }
    else
    {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpResponseCode).c_str());
    }
    
    http.end();
    return false;
}

void WiFi_Init()
{
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    int timeConnect = 0;
    while (WiFi.status() != WL_CONNECTED && timeConnect++ < 20) 
    {
        delay(500);
        Serial.print(".");
    }
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi not connected");
        return;
    }
    Serial.println("WiFi connected");
    WiFi.config(WiFi.localIP(), WiFi.gatewayIP(), WiFi.subnetMask(), dns);
}