#include "sender.h"

void Sender::setup()
{
    Wire.begin();//запускает I2C в режиме master
    Wire.setClock(100000);//устанавливает частоту SCL 100к√ц
    Wire.setWireTimeout(_wireTimeout, true);//устанавливаем timeout при достижении которого можно сбросить состо€ние TWI
    Wire.clearWireTimeoutFlag();
}


SendResult Sender::send(uint8_t* data)
{

    if (data == nullptr || _dataSize != 32) {
        return { 4, false };
    }

    Wire.clearWireTimeoutFlag();
    Wire.beginTransmission(_slaveAddr);

    Wire.write(data, _dataSize);

    uint8_t result = Wire.endTransmission();
    bool timedOut = Wire.getWireTimeoutFlag();

    return { result, timedOut };
}