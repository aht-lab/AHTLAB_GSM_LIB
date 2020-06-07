#include "aht_gsm_client.h"

void AHT_GSM_Client::setModem(AHT_GSM *modem)
{
    this->modem = modem;
}

int AHT_GSM_Client::connect(IPAddress ip, uint16_t port)
{
    char host[16];
    sprintf(host, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    return connect(host, port);
}

int AHT_GSM_Client::connect(const char *host, uint16_t port)
{
    this->state |= STATE_TCPCONNECTED;
    return this->modem->connectTCP(host, port);
}

size_t AHT_GSM_Client::write(uint8_t buff)
{
    return write(&buff, 1);
}

size_t AHT_GSM_Client::write(const uint8_t* buff, size_t size)
{
    DB_Print(F("TCP Write")); 
    // if (!this->modem->startSendTCP()) 
    // {
    //     return 0;
    // }
    size_t retval = this->modem->getUart()->write(buff, size);
    DB_Print(F("TCP Write retval: "));
    DB_Println(retval);
    // this->modem->getUart()->print(26);
    this->flush();

    return retval;
}

int AHT_GSM_Client::available()
{
    return (int)this->modem->available();
}

int AHT_GSM_Client::read()
{
    byte buff;
    buff = this->modem->getUart()->read();
    return buff;
}

int AHT_GSM_Client::read(uint8_t *buff, size_t size)
{
    byte rc = this->modem->getUart()->readBytes(buff, size);
    return rc;
}

int AHT_GSM_Client::peek()
{
    return -1;
}

void AHT_GSM_Client::flush()
{
    // this->modem->getUart()->flush();
    return;
}

void AHT_GSM_Client::stop()
{
    if (this->modem->disconnectTCP()) 
    {
        this->state &= ~STATE_TCPCONNECTED;
    }
}

uint8_t AHT_GSM_Client::connected()
{
    return (this->state & STATE_TCPCONNECTED);
}
