#include "WProgram.h"
#include "usb_dev.h"

uint8_t keys[KEYBOARD_SIZE] = {0};
void send();

void blink() {
  digitalWriteFast(13, HIGH);
  delay(16);
  digitalWriteFast(13, LOW);
  delay(16);
}

// See: https://github.com/tmk/tmk_keyboard/blob/master/keyboard/hhkb/doc/HHKB.txt
void select(int row, int col) {
  digitalWriteFast(2, row & 1 ? HIGH : LOW);
  digitalWriteFast(3, row & 2 ? HIGH : LOW);
  digitalWriteFast(4, row & 4 ? HIGH : LOW);

  digitalWriteFast(5, col & 1 ? HIGH : LOW);
  digitalWriteFast(6, col & 2 ? HIGH : LOW);
  digitalWriteFast(7, col & 4 ? HIGH : LOW);
}

void enable() {
  digitalWriteFast(8, LOW);
}

void disable() {
  digitalWriteFast(8, HIGH);
}

bool read() {
  return digitalReadFast(0) == LOW;
}

uint8_t matrix_prev[8] = {0};
uint8_t matrix[8] = {0};

uint8_t keymap[8][8] = {
 {  39,  28,  34,  30,  12,  37,  35,  14},  
 {  40,  41,  29,  16,  15,  17,  33,  13},  
 {  42,  43,  36,  31,  18,  19,  25,   0},  
 {  38,   0,   0,   2,   1,   4,   0,  52},  
 {  44,  45,  32,  20,  22,  21,  24,   0},  
 {   0,   0,  50,  48,   0,   5,   4,   0},  
 {  46,  47,  26,  27,   0,  23,   0,   0},  
 {   0,   0,   0,   0,   0,   0,   0,   0},  
};

bool getBit(uint8_t buffer[], uint8_t offset) {
  uint8_t row = offset / 8;
  uint8_t col = offset % 8;
  return buffer[row] & (1<<col);
}

void putBit(uint8_t buffer[], uint8_t offset, bool value) {
  uint8_t row = offset / 8;
  uint8_t col = offset % 8;
  if (value) {
    buffer[row] |= (1<<col);
  } else {
    buffer[row] &= ~(1<<col);
  }
}

void reboot() {
  __asm__ volatile("bkpt");
}

void scan() {
  for (uint8_t row = 0; row < 8; row++) {
    matrix_prev[row] = matrix[row];
    for (uint8_t col = 0; col < 8; col++) {
      select(row, col);
      delayMicroseconds(15);
      enable();
      delayMicroseconds(5);

      if (read()) {
        matrix[row] |= 1 << col;
      } else {
        matrix[row] &= ~(1 << col);
      }

      delayMicroseconds(5);
      disable();
      delayMicroseconds(75);
    }
  }
  bool changed = false;
  for (uint8_t row = 0; row < 8; row++) {
    if (matrix[row] != matrix_prev[row]) {
      changed = true;
      break;
    }
  }
  uint64_t thing = 0;
  if (changed) {
    for (uint8_t row = 0; row < 8; row++) {
      for (uint8_t col = 0; col < 8; col++) {
        putBit(keys, keymap[row][col], getBit(matrix, row * 8 + col));
      }
    }
    if (getBit(matrix, 1) && getBit(matrix, 4) && getBit(matrix, 5)) {
      delay(500);
      reboot();
    }
    send();
  }
}

extern "C" int main(void)
{
  pinMode(0, INPUT_PULLUP);
  for (uint8_t i = 1; i < 9; i++) {
  pinMode(i, OUTPUT);
  }
  pinMode(13, OUTPUT);

  blink();
  while (1) {
    scan();
  }
}

void send() {
  usb_packet_t *tx_packet;

  while (1) {
    if (!usb_configuration) {
      return;
    }
    tx_packet = usb_malloc();
    if (tx_packet) break;
    yield();
  }
  memcpy(tx_packet->buf, keys, KEYBOARD_SIZE);
  tx_packet->len = KEYBOARD_SIZE;
  usb_tx(KEYBOARD_ENDPOINT, tx_packet);
}
