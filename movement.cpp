#include "movement.h"


int sign(int a)
{
  if (a >= 0) return 1;
  return -1;
}


CheckersMovement::CheckersMovement(){
  digitalWrite(ENABLE_PIN, HIGH); // отключаем драйвер
  pinMode(ENABLE_PIN, OUTPUT); // настраиваем пин как выход

  for (byte axis = 0; axis < 3; axis++) {
    digitalWrite(STEP_PINS[axis], LOW);
    pinMode(STEP_PINS[axis], OUTPUT); // настраиваем пин как выход
    pinMode(DIR_PINS[axis], OUTPUT); // настраиваем пин как выход
  }
  delay(2000);
  digitalWrite(ENABLE_PIN, LOW); // включаем драйвера
  delay(10);

  moveMotor(takerY, backward, upDown);
  delay(1000);

  // инициализация магнита
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  disableMagnet();
}
  
void CheckersMovement::moveMotor(byte axis, bool direction, int steps) {
  
  if (axis == Z) {
    if (direction == forward) direction = backward;
    else direction = forward;
  }
  
  digitalWrite(DIR_PINS[axis], direction);
  delay(10);
  for (int i = 0; i < steps; i++) {
    digitalWrite(STEP_PINS[axis], HIGH);
    delayMicroseconds(50);
    digitalWrite(STEP_PINS[axis], LOW);
    delayMicroseconds(speed);
  }
}

 
void CheckersMovement::moveOneSquare(byte axis, bool direction) {

  moveMotor(axis, direction, oneSquare);
}

void CheckersMovement::enableMagnet()
{
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
}

void CheckersMovement::disableMagnet()
{
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
}

void CheckersMovement::takeChecker()
{
  moveMotor(takerY, forward, upDown);
  delay(200);
  enableMagnet();
  delay(1000);
  moveMotor(takerY, backward, upDown);
  checker_is_taken = true;
}

void CheckersMovement::putChecker()
{
  moveMotor(takerY, forward, upDown);
  delay(200);
  disableMagnet();
  delay(1000);
  moveMotor(takerY, backward, upDown);
  checker_is_taken = false;
}

void CheckersMovement::moveFromPosToPos(pos pos1, pos pos2, bool is_take) {
  int dx = pos2.x - pos1.x;
  int dz = pos2.z - pos1.z;

  int steps_count_x = abs(dx);
  int steps_count_z = abs(dz);
  
  bool direction_x;
  bool direction_z;

  if (sign(dx) > 0) direction_x = forward;
  else direction_x = backward;

  if (sign(dz) > 0) direction_z = forward;
  else direction_z = backward;


  moveMotor(X, direction_x, steps_count_x * oneSquare);
  moveMotor(Z, direction_z, steps_count_z * oneSquare);
  if (is_take) takeChecker();
  else putChecker();
}