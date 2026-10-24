#pragma once
#include <Wire.h>

struct SendResult {
    uint8_t status;
    bool success;
};

class Sender {
private:
    //Адрес ардуины на которую посылаем
    uint8_t _slaveAddr;

    //Размер массива данных для передачи
    uint8_t _dataSize;

    //Таймаут если за это время не отправится посылка, то соединение сбрасывается
    uint32_t _wireTimeout;

public:
    Sender(uint8_t slaveAddr, uint8_t dataSize, uint32_t wireTimeout)
    {
        _slaveAddr = slaveAddr;
        _dataSize = dataSize;
        _wireTimeout = wireTimeout;
    }

    //Инициализация соединения для отправки
    void setup();

    //Отправка данных
    SendResult send(uint8_t* data);
};
