#include "receiver.h"

Receiver* Receiver::_activeReceiver = nullptr;

Receiver::Receiver(uint8_t slaveAddr, uint32_t timeoutMs)
    : _slaveAddr(slaveAddr), _timeoutMs(timeoutMs) {
}

void Receiver::setup() {
    _activeReceiver = this;
    Wire.begin(_slaveAddr);
    Wire.onReceive(Receiver::onReceiveStatic);

    _lastReceiveMs = millis();
    _hasReceivedPacket = false;
    _dataAvailable = false;
}

void Receiver::onReceiveStatic(int dataSize) {
    if (_activeReceiver != nullptr) {
        _activeReceiver->handleReceive(dataSize);
    }
    else {
        while (Wire.available()) {
            (void)Wire.read();
        }
    }
}

void Receiver::handleReceive(int dataSize) {
    uint8_t incoming[DATA_SIZE];
    uint8_t count = 0;

    // Всегда вычитываем входящий буфер, в том числе при неправильной длине
    while (Wire.available()) {
        int value = Wire.read();
        if (count < DATA_SIZE) {
            incoming[count++] = static_cast<uint8_t>(value);
        }
    }

    // одна I2C-транзакция, ровно 32 байта fields
    if (dataSize != DATA_SIZE || count != DATA_SIZE) {
        return;
    }

    for (uint8_t i = 0; i < DATA_SIZE; ++i) {
        _fields[i] = incoming[i];
    }

    _lastReceiveMs = millis();
    _hasReceivedPacket = true;
    _dataAvailable = true;
}

bool Receiver::connectionLost() const {
    if (!_hasReceivedPacket) {
        return true;
    }

    return static_cast<uint32_t>(millis() - _lastReceiveMs) > _timeoutMs;
}

bool Receiver::available() const {
    return _dataAvailable;
}

bool Receiver::getData(uint8_t* dest) {
    if (dest == nullptr || !_dataAvailable) {
        return false;
    }

    noInterrupts();
    for (uint8_t i = 0; i < DATA_SIZE; ++i) {
        dest[i] = _fields[i];
    }
    _dataAvailable = false;
    interrupts();

    return true;
}