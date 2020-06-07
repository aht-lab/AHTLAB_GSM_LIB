#include "aht_sim800.h"

AHT_SIM800::AHT_SIM800(const AHT_GSM *gsm) 
{
    // sendAndReadResponse("ATE1", 1000);
}

void AHT_SIM800::hello() 
{
    DB_Println("AHT_SIM800");
}

bool AHT_SIM800::getIMEI(char* imei) 
{
    if(sendAndCheckReply("AT+CGSN", "OK", 2000, 1, 2) == AT_REPLY_FOUND)
    {
        int i = 0;
        while(_buffer[i] && _buffer[i] != '\r' && _buffer[i] != '\n')
        {
            imei[i] = _buffer[i];
            imei[++i] = '\0';
        }
    }
}

bool AHT_SIM800::getSimIMEI(char* simImei)
{
    if(sendAndCheckReply("AT+CIMI", "OK", 2000, 1, 2) == AT_REPLY_FOUND)
    {
        int i = 0;
        while(_buffer[i] && _buffer[i] != '\r' && _buffer[i] != '\n')
        {
            simImei[i] = _buffer[i];
            simImei[++i] = '\0';
        }
    }
}

bool AHT_SIM800::initLocation()
{
    if (sendAndCheckReply("AT+SAPBR=3,1,\"Contype\",\"GPRS\"", "OK", 2000, 2) != AT_REPLY_FOUND)
    {
        return 0;
    }

    if (sendAndCheckReply("AT+SAPBR=3,1,\"APN\",\"CMNET\"", "OK", 2000, 2) != AT_REPLY_FOUND)
    {
        return 0;
    }

    if (sendAndCheckReply("AT+SAPBR=0,1", "OK", 5000, 2) != AT_REPLY_FOUND)
    {
        DB_Println("AT+SAPBR=0,1 -> ERROR: Not yet connect");
    }

    if (sendAndCheckReply("AT+SAPBR=1,1", "OK", 5000, 2) != AT_REPLY_FOUND)
    {
        return 0;
    }

    return 1;
}

bool AHT_SIM800::getLocation(char* lat, char* lng) 
{
    if (!initLocation())
    {
        return 0;
    }
    
    if (!sendAndCheckReply("AT+CIPGSMLOC=1,1", "OK", 5000, 1, 2)) 
    {
        return 0;
    }

    bool success = Read_VARS("+CIPGSMLOC: *,%s,%s,", _buffer, lat, lng);
    DB_Println("lat: " + String(lat));
    DB_Println("lng: " + String(lng));

    return success && (atof(lat) != 0 && atof(lng) != 0);
}

bool AHT_SIM800::getCellId(char* cellId, char* lac)
{
    bool success;
    success = sendAndCheckReply("AT+CREG=2", "OK", 2000);
    success = sendAndCheckReply("AT+CREG?", "OK", 2000, 1, 2);
    if (!success) 
    {
        return false;
    }
    
    success = Read_VARS("+CREG: *,*,\"%s\",\"%s\"", _buffer, lac, cellId);
    DB_Println("lac: " + String(lac));
    DB_Println("cellId: " + String(cellId));
    
    return success;
}

bool AHT_SIM800::setupNTP(const char* ntpServer, const byte timezone)
{
    if (!initLocation()) 
    {
        return false;
    }

    if (sendAndCheckReply("AT+CNTPCID=1", "", 2000, 2) != AT_REPLY_FOUND)
    {
        return false;
    }

    print("AT+CNTP=");
    print(ntpServer);
    print(",");
    println(timezone * 4);
    if (readUntil(10000, "OK") != AT_REPLY_FOUND)
    {
        return false;
    }

    // TODO: if +CNTP: 1 fail: AT+SAPBR=0,1 AT+SAPBR=1,1
    if (sendAndCheckReply("AT+CNTP", "+CNTP: 1", 10000, 1, 2) != AT_REPLY_FOUND)
    {
        return false;
    }

    return true;
}

bool AHT_SIM800::getTimeNTP(uint16_t* year, uint16_t* month, uint16_t* day, uint16_t* hour,
                            uint16_t* minute, uint16_t* second, uint16_t* timezone)
{
    if (!sendAndCheckReply("AT+CCLK?", "OK", 5000, 1, 2)) 
    {
        return 0;
    }

    uint16_t tyear, tmonth, tday, thour, tminute, tsecond, ttimezone;

    bool success = Read_VARS("+CCLK: \"%d/%d/%d,%d:%d:%d+%f\"", _buffer, 
                            &tyear, &tmonth, &tday, &thour, &tminute, &tsecond, &ttimezone);

    *year = tyear;
    *month = tmonth;
    *day = tday;
    *hour = thour;
    *minute = tminute;
    *second = tsecond;
    *timezone = ttimezone;

    return success;
}

char AHT_SIM800::numSMS(byte status) 
{
    int smsNum = 0;
    char cmdAT[25];
    
    switch(status)
    {
        case SMS_STT_ALL:
            sprintf(cmdAT, "AT+CMGL=\"%s\"", "ALL");
            break;
    }
    
    if(sendAndCheckReply("AT+CMGF=1", "OK", 1000, 2) != AT_REPLY_FOUND)
    {
        return -1;
    }

    if (sendAndCheckReply("AT+CSCS=\"UCS2\"", "OK", 1000, 2) != AT_REPLY_FOUND)
    {
        return -1;
    }
    
    _uart->println(cmdAT);

    byte segmentRetVal      = 0;
    const uint32_t t        = millis();
    const uint32_t TIMEOUT  = 35000L;
    while(segmentRetVal != SEG_STOP)
    {
        segmentRetVal = readSegment("OK", 2000);
        if(segmentRetVal == SEG_END && strstr(_buffer, "+CMGL:"))
        {
            char* p1 = strchr((char*)_buffer, ':');
            char* p2 = p1 + 1;
            p2 = strchr((char*)p2, ':');
            while(p2 != nullptr) 
            {
                p1 = p2;
                p2 = p2 + 1;
                char* p2 = strchr((char*)p2, ':');
            }
            if (p1 == NULL) continue;
            smsNum = atoi((p1 + 1));
            DB_Println("smsIndex: " + String(smsNum));
        }
        else if(segmentRetVal != SEG_END &&  millis() - t > TIMEOUT)
        {
            break;
        }
    }

    return smsNum;
}

char AHT_SIM800::readSMS(uint8_t index, char* phone, char* msg)
{
    char retVal = 0;
    char cmdAT[13];
    sprintf(cmdAT, "AT+CMGR=%d", index);
    
    if(sendAndCheckReply("AT+CMGF=1", "OK", 1000, 2) != AT_REPLY_FOUND)
    {
        return SMS_READ_FAIL;
    }

    if(sendAndCheckReply("AT+CSCS=\"UCS2\"", "OK", 1000,2) != AT_REPLY_FOUND)
    {
        return SMS_READ_FAIL;
    }
        
    if(sendAndCheckReply(cmdAT, "+CMGR:", 6000, 2) != AT_REPLY_FOUND)
    {
        return SMS_READ_FAIL;
    }
    
    char phoneUCS2[21];
    char* p1 = strchr((char*)_buffer, ',');
    p1 = p1 + 2;
    char* p2 = strchr((char*)p1, ',');
    p2 = p2 - 1;
    *p2 = 0;
    strcpy(phoneUCS2, p1);
    convertUCS2(phoneUCS2, phone);

    char msgUCS2[1024];
    p2 = p2 + 1;
    p1 = strchr((char*)p2, '\n');
    p1 = p1 + 1;
    p2 = strchr((char*)p1, '\r');
    if(p2 != nullptr)
    {
        *p2 = 0;
    }
    strcpy(msgUCS2, p1);
    convertUCS2(msgUCS2, msg);

    return SMS_READ_SUCCESS;
}

char AHT_SIM800::deleteSMS(uint8_t index)
{
    char pattern[] = "AT+CMGD=%d,0";
    char at[13];
    sprintf(at, pattern, index);
    return sendAndCheckReply(at, "OK", 5000, 2) == AT_REPLY_FOUND;
}

char AHT_SIM800::deleteAllSMS() 
{
    return sendAndCheckReply("AT+CMGD=1,4", "OK", 25000, 2) == AT_REPLY_FOUND;
}

void AHT_SIM800::printPhone(const char* phone)
{
    _uart->print(F("AT+CMGS=\""));
    unsigned char i=0;
    while(phone[i])
    {
        _uart->print(F("00"));
        _uart->print(phone[i],HEX);
        i++;
    }
    _uart->println(F("\""));
}

void AHT_SIM800::printSMS(const char* data)
{
    unsigned char i=0;
    String buf="";
    while(data[i])
    {
        unsigned char c = data[i]&0xFF;
        if(c==0xE0)
        {
            _uart->print(F("0E"));
            DB_Print(F("OE"));
            i++;
            c = data[i];
            if(c == 0xB8)
            {
                i++;
                c = data[i]-0x80;
                if(c <= 0x0F)
                {
                  _uart->print(F("0"));
                  DB_Print(F("0"));        
                }
                buf = String(c,HEX);
                buf.toUpperCase();
                _uart->print(buf);
                DB_Print(buf); 
                }
            else
            {
                i++;
                c = data[i]-0x40;
                if(c <= 0x0F)
                {
                    _uart->print(F("0"));
                    DB_Print(F("0")); 
                }
                buf = String(c,HEX);
                buf.toUpperCase();
                _uart->print(buf);
                DB_Print(buf); 
            }     
        }
        else
        {
            _uart->print(F("00"));
            DB_Print(F("00")); 
            if(c == 0x0A)
            {
                _uart->print("0A");
                DB_Print(F("0A")); 
            }
            else if(c == 0x0D)
            {
                _uart->print("0D");
               DB_Print(F("0D")); 
            }
            else
            {
                buf = String(c,HEX);
                buf.toUpperCase();
                _uart->print(buf);
                DB_Print(buf); 
            }
        
        }
        i++;
    }
    DB_Print("\n");
}

void AHT_SIM800::printlnSMS(const char* data) {
    printSMS(data);
    printSMS("\r\n");
}

char AHT_SIM800::sendSMS(const char* phone, const char* content)
{
    if(sendAndCheckReply("AT+CMGF=1", "OK") != AT_REPLY_FOUND)
    {
        return 0;
    }
    if(sendAndCheckReply("AT+CSCS=\"UCS2\"", "OK", 1500) != AT_REPLY_FOUND)
    {
        return 0;
    }
    
    printPhone(phone);
    if(readResponse(2000, ">") == AT_REPLY_FOUND)
    {
        printlnSMS(content);
        _uart->write(0x1A);
        
        return readUntil(10000, "OK") == AT_REPLY_FOUND;
    }
    return 0;
}

bool AHT_SIM800::setupCall()
{
    sendAndReadResponse("AT+CLCC?", 1000);
    return sendAndCheckReply("AT+CLCC=1", "OK", 2000, 2) == AT_REPLY_FOUND;
}

uint16_t AHT_SIM800::call(const char* phone, uint16_t timeWait, uint16_t timeCallLimit)
{
    char AT[16];
    sprintf(AT, "ATD%s;", phone);

    println(AT);
    if(readUntil(1000, "OK") != AT_REPLY_FOUND)
    {
        return 0;
    }

    uint32_t    t           = millis();
    uint32_t    timeCall    = 0;
    bool        isActive    = false;
    CALL_STATUS status      = DIALING;
    while(status != DISCONNECT)
    {
        delay(1);
        if(available() && readUntil(1000, "\r\n") == AT_REPLY_FOUND)
        {
            uint8_t stat;
            if(Read_VARS("+CLCC: *,*,%d", _buffer, &stat))
            {
                DB_Print("stat: ");
                DB_Println(stat);
                switch(stat)
                {
                    case 0:
                        status = ACTIVE;
                        t = millis();
                        isActive = true;
                        DB_Println("ACTIVE");

                        continue;

                    case 2:
                        status = DIALING;
                        t = millis();
                        DB_Println("DIALING");

                        continue;

                    case 3:
                        status = ALERTING;
                        t = millis();
                        DB_Println("RING");

                        continue;

                    case 6:
                        status = DISCONNECT;
                        sendAndReadResponse("ATH", 2000);
                        DB_Println("DISCONNECT");

                        continue;

                    default: continue;
                }
            }
        }
        else switch(status)
        {
            case DIALING:
                if(millis() - t > 20000L)
                {
                    sendAndReadResponse("ATH", 2000);
                    status = DISCONNECT;
                }
                continue;
            
            case ALERTING:
                if(millis() - t > timeWait*1000)
                {
                    sendAndReadResponse("ATH", 2000);
                    status = DISCONNECT;
                }
                continue;

            case ACTIVE:
                timeCall = (millis() - t) / 1000;
                if(timeCall > timeCallLimit)
                {
                    sendAndReadResponse("ATH", 2000);
                    status = DISCONNECT;
                }
                continue;
        }
    }

    return timeCall;
}

char AHT_SIM800::call(const char* phone)
{
    char AT[16];
    sprintf(AT, "ATD%s;", phone);
    return sendAndCheckReply(AT, "OK", 1000, 2) == AT_REPLY_FOUND;
}

char AHT_SIM800::handup()
{
    return sendAndCheckReply("ATH", "OK", 1000, 2) == AT_REPLY_FOUND;
}

char AHT_SIM800::phoneActiveSTT()
{
    return sendAndCheckReply("AT+CPAS", "+CPAS: 4", 1000, 1) == AT_REPLY_FOUND;
}

char AHT_SIM800::inMultiConnection()
{
    char ret = sendAndCheckReply("AT+CIPMUX?", "OK", 1000, 2);
    if (ret != AT_REPLY_FOUND)
    {
        return ret;
    }

    char* colon = strchr(_buffer, ':');
    if (colon == nullptr) 
    {
        return AT_ERROR;
    }
    uint8_t mode = atoi((const char*) (colon + 2));
    DB_Print("Mode: ");
    DB_Println(mode);

    return mode;
}

char AHT_SIM800::attackGPRS(const char* domain, const char* user, const char* pass)
{
    sendAndReadResponse("AT+CIPSHUT", 2000);

    // TODO: +CIPMUX? -> +CIPMUX: (0,1) -> 1 -> 0
    char ret = inMultiConnection();
    if (ret < 0)
    {
        return 0;
    }

    if (ret && sendAndReadResponse("AT+CIPMUX=0", "OK", 2000, 2) != AT_REPLY_FOUND)
    {
        return 0;
    }

    // TODO: +CGACT? -> +CGACT: <cid>,<state>[<CR><LF>+CGACT:<cid>,<state>…] -> state: 1 -> activated
    sendAndReadResponse("AT+CGACT?", 2000);
    if(sendAndCheckReply("AT+CGATT=1", "OK", 8000, 3) != AT_REPLY_FOUND)
    {
        return 0;
    }

    if (sendAndCheckReply("AT+CIPQSEND=0", "OK", 2000) != AT_REPLY_FOUND)
    {
        return 0;
    }

    // TODO: disable?
    sendAndReadResponse("AT+CIPRXGET=0", 2000);

    print("AT+CSTT=\"");
    print(domain);
    print("\",\"");
    print(user);
    print("\",\"");
    print(pass);
    print("\"\r");

    if(readUntil(5000, "OK") != AT_REPLY_FOUND)
    {
        return 0;
    }

    if(sendAndCheckReply("AT+CIICR", "OK", 7000, 2) != AT_REPLY_FOUND)
    {
        return 0;
    }

    if(sendAndReadResponse("AT+CIFSR", 2000) == AT_ERROR)
    {
        return 0; 
    }

    return 1;
}

char AHT_SIM800::dettachGPRS()
{
    return sendAndCheckReply("AT+CGATT=0", "OK", 5000, 2) == AT_REPLY_FOUND;
}

char AHT_SIM800::connectTCP(const char* server, uint16_t port)
{
    DB_Println("connectTCP");

    print("AT+CIPSTART=\"TCP\",\"");
    print(server);
    print("\",\"");
    print(port);
    print("\"\r");

    return readUntil(15000, "CONNECT OK") == AT_REPLY_FOUND;
}

char AHT_SIM800::startSendTCP()
{
    println("AT+CIPSEND");
    return readUntil(5000, ">") == AT_REPLY_FOUND;
}

char AHT_SIM800::disconnectTCP()
{
    return sendAndCheckReply("AT+CIPCLOSE=0", "OK", 2000, 2) == AT_REPLY_FOUND;
}

char AHT_SIM800::requestPost(const char* server, const char* path, int port, const char* data, char* body, int len)
{
    char end_c[2], buffer[8];
    end_c[0]=0x1a;
    end_c[1]='\0';
    
    if (!connectTCP(server, port))
    {
        return 0;
    }

    if (!startSendTCP()) 
    {
        return 0;
    }

    print("POST ");
    print(path);
    print(" HTTP/1.1\r\nHost: ");
    print(server);
    print("\r\n");
    print("User-Agent: Arduino\r\n");
    print("Content-Type: application/x-www-form-urlencoded\r\n");
    print("Content-Length: ");
    itoa(strlen(data), buffer, 10);
    print(buffer);
    print("\r\n\r\n");
    print(data);
    print("\r\n\r\n");
    print(end_c);

    if(readUntil(6000, "SEND OK") == AT_REPLY_FOUND)
    {
        if(readResponse(10000, "200 OK"))
        {
            DB_Println("HTTP STATUS: 200");
            // process header and body
            int i = 0;
            const char* pbody = strstr(_buffer, "\r\n\r\n");
            if(pbody != nullptr)
            {
                pbody += 4;
            }
            while(*pbody && *pbody != '\r' && *pbody != '\n' && i < len)
            {
                body[i++] = *pbody++;
                body[i] = '\0';
            }
        }
        else
        {
            return 0;
        }
    }
    else
    {
        disconnectTCP();
    }
    
    return 1;
}

char AHT_SIM800::requestGet(const char* server, const char* path, int port, char* body, int len)
{
    char end_c[2];
    end_c[0]=0x1a;
    end_c[1]='\0';
    
    if(!connectTCP(server, port))
    {
        return 0;
    }

    if (!startSendTCP()) 
    {
        return 0;
    }

    print("GET ");
    print(path);
    print(" HTTP/1.0\r\nHost: ");
    print(server);
    print("\r\n");
    print("User-Agent: Arduino");
    print("\r\n\r\n");
    print(end_c);
    print("\r");

    if(readUntil(6000, "SEND OK") == AT_REPLY_FOUND)
    {
        if(readResponse(10000, "200 OK"))
        {
            DB_Println("HTTP STATUS: 200");
            // process header and body
            int i = 0;
            const char* pbody = strstr(_buffer, "\r\n\r\n");
            if(pbody != nullptr)
            {
                pbody += 4;
            }
            while(*pbody && *pbody != '\r' && *pbody != '\n' && i < len)
            {
                body[i++] = *pbody++;
                body[i] = '\0';
            }
        }
        else
        {
            return 0;
        }
    }
    else
    {
        disconnectTCP();
    }
    
    return 1;
}
