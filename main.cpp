#include "WProgram.h"
#include "usb_dev.h"
#include "hhkb.h"
#include "rawhid.h"

uint8_t keys[KEYBOARD_SIZE] = {0};
void send();

void blink() {
  digitalWriteFast(13, HIGH);
  delay(16);
  digitalWriteFast(13, LOW);
  delay(16);
}

uint8_t keymap[8][8] = {
 {  39,  28,  34,  30,  12,  37,  35,  14},  
 {  40,  41,  29,  16,  15,  17,  33,  13},  
 {  42,  43,  36,  31,  18,  19,  25,   0},  
 {  38,  49,  51,   4,   1,   2,   3,  52},  
 {  44,  45,  32,  20,  22,  21,  24,   0},  
 {  58,  61,  50,  48,   0,   5,   6,   7},  
 {  46,  47,  26,  27,  59,  23,  62,   0},  
 {  53,  54,  56,  55,  60,  64,  63,   0},  
};

bool getBit(uint64_t vector, uint8_t offset) {
  return (vector & (UINT64_C(1) << offset)) > 0;
}

void putBit(uint64_t& vector, uint8_t offset, bool value) {
  if (value) {
    vector |= 1 << offset;
  } else {
    vector &= ~(1 << offset);
  }
}

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

extern "C" int main(void)
{
  pinMode(13, OUTPUT);

  rawhid raw;
  hhkb keyboard;
  keyboard.init();

  while (1) {
    uint32_t now = micros();
    if (keyboard.scan()) {
      uint64_t matrix = keyboard.matrix();
      for (uint8_t row = 0; row < 8; row++) {
        for (uint8_t col = 0; col < 8; col++) {
          putBit(keys, keymap[row][col], getBit(matrix, row * 8 + col));
        }
      }
      if (getBit(matrix, 1) && getBit(matrix, 4) && getBit(matrix, 5)) {
        reboot();
      }
      send();
      memcpy(raw.tx, &matrix, 8);
      memcpy(raw.tx + 8, &now, 4);
      raw.send();
    }
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
