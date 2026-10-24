#pragma once
#include <Arduino.h>
#include <Wire.h>

class Receiver {
public:
    Receiver(uint8_t slaveAddr, uint32_t timeoutMs);
    void setup();

    // true, если первый корректный пакет ещё не пришёл или истёк timeout, для проверки на обрыв соединения
    bool connectionLost() const;

    // true, если есть новые данные, которые ещё не забрал вызывающий код
    bool available() const;

    // Копирует 32 байта в dest; возвращает false, если данных нет или dest == nullptr
    bool getData(uint8_t* dest);

private:
    static constexpr uint8_t DATA_SIZE = 32;

    uint8_t _slaveAddr;
    uint32_t _timeoutMs;
    uint8_t _fields[DATA_SIZE] = {};

    volatile bool _dataAvailable = false;
    volatile bool _hasReceivedPacket = false;
    volatile uint32_t _lastReceiveMs = 0;

    // Wire.onReceive требует обычную функцию, поэтому callback перенаправляется к активному объекту Receiver
    static Receiver* _activeReceiver;
    static void onReceiveStatic(int dataSize);
    void handleReceive(int dataSize);
};
