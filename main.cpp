#include "WProgram.h"

extern "C" int main(void)
{
  pinMode(13, OUTPUT);
  int i = 0;
  while (1) {
    if (i == 5) { Keyboard.press(65); }
    if (i == 6) { Keyboard.releaseAll(); }
    i++;
    digitalWriteFast(13, HIGH);
    delay(500);
    digitalWriteFast(13, LOW);
    delay(500);
  }
}

