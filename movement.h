#ifndef MOVEMENT
#define MOVEMENT

#include "Arduino.h"
// X вперед
// Y вверх
// Z вправо
  const byte STEP_PINS[3] = {5, 6, 7};  // X, Z, Y
  const byte DIR_PINS[3]  = {2, 3, 4};

  const byte ENABLE_PIN = 8;

  const byte X = 0;
  const byte Z = 1;
  const byte takerY = 2;

  const bool forward = HIGH;
  const bool backward = LOW;

  const int oneSquare = 3400;
  const int upDown = 1500;

  const int IN1 = 12;
  const int IN2 = 13;

struct pos
{
    short x;
    short z;
};

class CheckersMovement {
private:
  int speed = 200; // скорость
  bool checker_is_taken = false;
  
public:
  CheckersMovement();
  
  void moveMotor(byte axis, bool direction, int steps);

  void moveOneSquare(byte axis, bool direction);

  void enableMagnet();

  void disableMagnet();

  void takeChecker();

  void putChecker();

  void moveFromPosToPos(pos pos1, pos pos2, bool is_take);

};
#endif
