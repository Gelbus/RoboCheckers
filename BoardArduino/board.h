#pragma once
#include <Arduino.h>
#include "sender.h"

class Board {
private:
    //ѕины с которых снимаетс€ значение магнитного пол€ на клетках доски
    const uint8_t MUX1_PIN = A0;
    const uint8_t MUX2_PIN = A1;

    //ѕины на которые подаетс€ код канала
    const uint8_t S0_PIN = 2;
    const uint8_t S1_PIN = 3;
    const uint8_t S2_PIN = 4;
    const uint8_t S3_PIN = 5;

    //»нтервал обновлени€ и отправки данных с доски в милисекундах
    const uint32_t SEND_INTERVAL_MS = 10;

    //—ледующее врем€ сн€ти€ сигналов с датчиков и отправки
    uint32_t _nextCheckAndSendTime;

    //ѕорог по которому определ€ем присутствие шашки на клетке
    //ѕри помощи него можно регулировать чувствительность доски
    const uint16_t THRESHOLD = 512;

    // при true -> 1 в fields - высокое значение с сенсора, 0 в fields - низкое значение сигнала с сенсора
    // при false -> 1 в fields - низкое значение с сенсора, 0 в fields - высокое значение сигнала с сенсора
    const bool PRESENCE_WHEN_HIGH = false;

    Sender _sender = Sender(0x08, 32, 3000);
    //ћассив значений с датчиков на пол€х доски
    uint8_t fields[32];

    //”станавливает на пины двоичный код канала (от 0 до 15)
    void selectChannel(uint8_t channel);

    //ѕровер€ет по значению датчика зан€то ли поле доски
    byte isFieldOccuped(int32_t value);

    //ќбновл€ет информацию с датчиков
    void readSensors();
public:
    //”становка пинов дл€ выбора каналов 
    void setup();

    void trackFieldsAndSend();
};