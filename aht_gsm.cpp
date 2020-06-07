#include "aht_gsm.h"

AHT_GSM::AHT_GSM() 
{
    
}

#ifndef ESP32
AHT_GSM::AHT_GSM(SoftwareSerial *uart) 
{
    _uart = uart;
    uartType = SOFTWARE;
}
#endif

AHT_GSM::AHT_GSM(HardwareSerial *uart) 
{
    _uart = uart;
    uartType = HARDWARE;
}

void AHT_GSM::begin(uint32_t baudrate)
{
    if (uartType == HARDWARE) 
    {
        HardwareSerial* _uartTemp = (HardwareSerial*) _uart;
        _uartTemp->begin(baudrate); 
    } 
    else 
    {
#ifndef ESP32
        SoftwareSerial* _uartTemp = (SoftwareSerial*) _uart;
        _uartTemp->begin(baudrate);
#endif
    }
    _baudrate = baudrate;
}

bool AHT_GSM::begin() 
{
    int baudrateSize = 2;
    uint32_t baudrates[] = {9600, 115200};

    DB_Println("(*)DETECT BAUDRATE");
    for (int i = 0; i < baudrateSize; i++) 
    {
        DB_Println("-Try baudrate: " + String(baudrates[i]));

        int timeTest = 0, MAX_TIME = 5;
        begin(baudrates[i]);
        while (timeTest++ < MAX_TIME)
        {
            DB_Println("-Time try: " + String(timeTest));
            if (sendAndCheckReply("AT", "OK"))
            {
                readResponse(10000);
                DB_Println(F("=>BAUDRATE OK"));
                return true;
            }
        }
    }

    // enable echo
    sendAndReadResponse("ATE1", 1000);
    
    return false;
}

Stream* AHT_GSM::getUart() 
{
    return _uart;
}

uint16_t AHT_GSM::getBaudrate()
{
    return _baudrate;
}

void AHT_GSM::setUart(Stream* uart) 
{
    _uart = uart;
}

void AHT_GSM::DB_Buffer(uint16_t index) 
{
    DB_Println("----AT RESPONSE----");
    for (int i = 0; i < index; i++) 
    {
        DB_Print(_buffer[i]);
    }
    DB_Println();
    DB_Println("-------------------");
}

void AHT_GSM::hello() 
{
    DB_Println("AHT_GSM");
}

void AHT_GSM::print(const char* at)
{
    _uart->print(at);
}

void AHT_GSM::print(int val)
{
    _uart->print(val);
}


void AHT_GSM::println(const char* at)
{
    _uart->println(at);
}

void AHT_GSM::println(int val)
{
    _uart->println(val);
}

bool AHT_GSM::available()
{
    return _uart->available();
}

char AHT_GSM::WaitForReply(uint16_t timeout)
{
    uint16_t timeWait = 0;
    while(_uart->available() == 0) 
    {
        delay(1);
        if(++timeWait > timeout) 
        { 
            DB_Print("\nNO RESPONSE");
            return AT_NO_RESPONSE;
        }
    }
    return AT_AVAILABLE;
}

char AHT_GSM::readSegment(const char* strStop, uint16_t timeout)
{
    char retVal = 0;
    uint16_t index = 0;
    _buffer[0] ='\0';
    
    while(timeout--) 
    {
        if (retVal > 0) break;

        while(_uart->available())
        {
            if(index > BUFFER_SIZE - 2)
            { 
                retVal = SEG_END;
                break;
            }
            
            char c = _uart->read();
            if (index == 0 && c == '\n') 
            {
                continue;
            }
            
            _buffer[index++] = c;
            _buffer[index] = '\0';
            if(c == '\n')
            {
                if(strstr(_buffer, "\r\n\r\n"))
                {
                    retVal = SEG_END;
                    break;
                }
                if(strstr(_buffer, strStop))
                {
                    retVal = SEG_STOP;
                    break;
                }
            }
        }
        delay(1);
    }

    if(retVal == 0 && index == 0)
    {
        retVal = AT_NO_RESPONSE;
    }
    else
    {
        DB_Buffer(index);   
    }

    return retVal;
}

char AHT_GSM::readResponse(uint16_t timeout)
{
    char retVal = 0;
    uint16_t timeWait = 0, index = 0;
    _buffer[0] = '\0';

    while(timeout--) 
    {
        delay(1);
        if(_uart->available())
        {
            if (index > BUFFER_SIZE - 2)
            { 
                _uart->read();
                continue;
            }            
            char c = _uart->read();
            if (index == 0 && c == '\n') 
            {
                continue;
            }
            _buffer[index++] = c;
            _buffer[index] = '\0';
        }
    }

    if(strstr(_buffer, "ERROR")) 
    {
        DB_Println(F("AT_ERROR"));
        retVal = AT_ERROR;
    }
    else if(index == 0)
    {
        retVal = AT_NO_RESPONSE;
    }
    else
    {
        DB_Buffer(index);
        retVal = AT_READ_OK;
    }
    
    _bufferLen = index;

    return retVal;
}

char AHT_GSM::readResponse(const char* reply, uint8_t line, uint16_t timeout)
{
    char retVal = 0;
    bool foundReply = false;
    uint16_t index = 0, linesFound = 1, currentIndex = 0;
    char lineContent[512];

    _buffer[0] = '\0';
    
    while(timeout--) 
    {
        delay(1);
        if(_uart->available())
        {
            if(foundReply)
            {
                _uart->read();
                continue;
            }
            
            if(index > BUFFER_SIZE - 2)
            { 
                _uart->read();
                continue;
            }            
            char c = _uart->read();
            if(index == 0 && c == '\n' && currentIndex == 0 && linesFound == 1) 
            {
                continue;
            }

            if(linesFound != line)
            {
                lineContent[currentIndex++] = c;
                lineContent[currentIndex] = '\0';
            }
            else
            {
                _buffer[index++] = c;   
                _buffer[index] = '\0'; 
            }
            
            if(c == '\n')
            {
                DB_Println(lineContent);
                if(linesFound != line) 
                {
                    foundReply = strstr(lineContent, reply);
                }
                linesFound++;
                currentIndex = 0;
            }
        }
    }
    
    if(foundReply)
    {
        retVal = AT_REPLY_FOUND;
    }
    else
    {
        DB_Println(F("AT_REPLY_NOT_FOUND"));
        retVal = AT_REPLY_NOT_FOUND;
        if(index == 0)
        {
            retVal = AT_NO_RESPONSE;
        }
        else if(strstr(_buffer, "ERROR"))
        {
            DB_Println(F("AT_ERROR"));
            retVal = AT_ERROR;
        }
    }

    if (retVal != AT_NO_RESPONSE)
    {
        DB_Buffer(index);
    }
    
    _bufferLen = index;
    
    return retVal;
}

char AHT_GSM::readResponse(uint16_t timeout, const char* reply)
{
    char retVal = 0;
    bool foundReply = false;
    uint16_t index = 0;

    _buffer[0] = '\0';
    
    while(timeout--) 
    {
        delay(1);
        if(_uart->available())
        {
            if(foundReply)
            {
                while(_uart->available())
                {
                    if(index > BUFFER_SIZE - 2)
                    { 
                        _uart->read();
                    }
                    else
                    {
                        char c = _uart->read();
                        _buffer[index++] = c;   
                        _buffer[index] = '\0';
                    }
                }
                continue;
            }
            
            if(index > BUFFER_SIZE - 2)
            { 
                _uart->read();
                continue;
            }            
            char c = _uart->read();
            if(index == 0 && c == '\n') 
            {
                continue;
            }

            _buffer[index++] = c;   
            _buffer[index] = '\0'; 
            
            if(c == '\n')
            {
                foundReply = strstr(_buffer, reply);
            }
        }
    }
    
    if(foundReply)
    {
        retVal = AT_REPLY_FOUND;
    }
    else
    {
        DB_Println(F("AT_REPLY_NOT_FOUND"));
        retVal = AT_REPLY_NOT_FOUND;
        if(index == 0)
        {
            retVal = AT_NO_RESPONSE;
        }
        else if(strstr(_buffer, "ERROR"))
        {
            DB_Println(F("AT_ERROR"));
            retVal = AT_ERROR;
        }
    }

    if (retVal != AT_NO_RESPONSE)
    {
        DB_Buffer(index);
    }
    
    _bufferLen = index;
    
    return retVal;
}

char AHT_GSM::readUntil(uint16_t timeout, const char* reply)
{
    char retVal = 0;
    bool foundReply = false;
    uint16_t index = 0;

    _buffer[0] = '\0';
    
    while(timeout--) 
    {
        delay(1);
        if(_uart->available())
        {
            if(foundReply)
            {
                break;
            }
            
            if(index > BUFFER_SIZE - 2)
            { 
                _uart->read();
                continue;
            }            
            char c = _uart->read();
            if(index == 0 && (c == '\n' || c == '\r'))
            {
                continue;
            }

            _buffer[index++] = c;   
            _buffer[index] = '\0'; 
            
            foundReply = strstr(_buffer, reply);
        }
    }
    
    if(foundReply)
    {
        retVal = AT_REPLY_FOUND;
    }
    else
    {
        DB_Println(F("AT_REPLY_NOT_FOUND"));
        retVal = AT_REPLY_NOT_FOUND;
        if(index == 0)
        {
            retVal = AT_NO_RESPONSE;
        }
        else if(strstr(_buffer, "ERROR"))
        {
            DB_Println(F("AT_ERROR"));
            retVal = AT_ERROR;
        }
        DB_Buffer(_bufferLen);
    }

    if (retVal != AT_NO_RESPONSE)
    {
        DB_Buffer(index);
    }
    
    _bufferLen = index;
    
    return retVal;
}

char AHT_GSM::sendAndReadResponse(const char* command, const char* reply, uint8_t line, uint16_t timeout)
{
    char retVal = 0;
    
    DB_Print("\nAT: ");
    DB_Println(command);
    _uart->println(command);
    
    retVal = WaitForReply(timeout);
    if (retVal == AT_NO_RESPONSE)
    {
        DB_Println(F("AT_NO_RESPONSE"));
        return retVal;
    }
    retVal = readResponse(reply, line, timeout);

    return retVal;
}

char AHT_GSM::sendAndReadResponse(const char* command, uint16_t timeout) 
{
    char retVal = 0;
    
    DB_Print("\nAT: ");
    DB_Println(command);
    _uart->println(command);
    
    retVal = WaitForReply(timeout);
    if (retVal == AT_NO_RESPONSE)
    {
        DB_Println(F("AT NO RESPONSE"));
        return retVal;
    }
    retVal = readResponse(timeout);
    
    return retVal;
}

char AHT_GSM::sendAndCheckReply(const char* command, const char* reply, uint16_t timeout, uint8_t timeTry, uint8_t line) 
{
    char retVal = 0;
    
    while (retVal != AT_OK && timeTry > 0) 
    {
        --timeTry;
        
        if (line > 0)
        {
            if (sendAndReadResponse(command, reply, line, timeout) == AT_REPLY_FOUND) 
            {
                retVal = AT_REPLY_FOUND;
                break;
            }
        }
        else
        {
            if(sendAndReadResponse(command, timeout) != AT_READ_OK) 
            {
                continue;
            }
            
            if(strstr(_buffer, reply) != nullptr)
            {
                retVal = AT_REPLY_FOUND;
                break;
            }
            else
            {
                retVal = AT_REPLY_NOT_FOUND;
                break;
            }
        }
    }

    return retVal;
}

GSM_TYPE AHT_GSM::detectGSM(const Stream* uart) 
{
    DB_Println(F("\n(*)DETECT GSM"));
    sendAndCheckReply("AT+CGMM", "OK");

    if (strstr(_buffer, "SIMCOM_SIM800A") != nullptr)
    {
        DB_Println(F("=>GSM: SIM800A"));
    }
    else if (strstr(_buffer, "UC15") != nullptr)
    {
        DB_Println(F("=>GSM: UC15"));
    }
    else 
    {
        DB_Println(F("=>GSM: NOT_FOUND"));
    }
    
    return SIM800;
}
