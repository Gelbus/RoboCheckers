#include "board.h"

Board board;

void setup() 
{
  board.setup();
}

void loop() 
{
  board.trackFieldsAndSend();
}
