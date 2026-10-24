#include "movement.h"

void setup() {
  CheckersMovement checkers;
  pos pos1, pos2;
  pos1.x = 0;
  pos1.z = 0;

  pos2.x = 10;
  pos2.z = 0;


  //checkers.takeChecker();
  //delay(1000);
  //checkers.moveFromPosToPos(pos1, pos2, true);
  checkers.moveFromPosToPos(pos2, pos1, false);
  

  digitalWrite(ENABLE_PIN, HIGH);
}

void loop() {
  // Для повторного теста нажми RESET.
}
