#include "board.h"

void Board::selectChannel(uint8_t channel)
{
    digitalWrite(S0_PIN, channel & 1);
    digitalWrite(S1_PIN, (channel >> 1) & 1);
    digitalWrite(S2_PIN, (channel >> 2) & 1);
    digitalWrite(S3_PIN, (channel >> 3) & 1);
}

byte Board::isFieldOccuped(int32_t value)
{
    return PRESENCE_WHEN_HIGH ? (value > THRESHOLD) : (value < THRESHOLD);
}

void Board::setup()
{
    pinMode(S0_PIN, OUTPUT);
    pinMode(S1_PIN, OUTPUT);
    pinMode(S2_PIN, OUTPUT);
    pinMode(S3_PIN, OUTPUT);
    _sender.setup();
    _nextCheckAndSendTime = millis();
}

void Board::readSensors()
{
    int raw1;
    int raw2;

    // Выбираем канал один раз для обоих мультиплексоров
    for (uint8_t channel = 0; channel < 16; channel++)
    {
        selectChannel(channel);
        delayMicroseconds(5);

        raw1 = analogRead(MUX1_PIN);
        raw2 = analogRead(MUX2_PIN);

        fields[channel] = isFieldOccuped(raw1);
        fields[channel + 16] = isFieldOccuped(raw2);
    }
}

void Board::trackFieldsAndSend()
{
    uint32_t now = millis();
    if (now - _nextCheckAndSendTime < 0)
    {
        return;
    }
    _nextCheckAndSendTime = now + SEND_INTERVAL_MS;

    readSensors();

    auto [result, timedOut] = _sender.send(fields);

    if (result != 0 || timedOut)
    {
        readSensors();
        _nextCheckAndSendTime = millis();
    }
}