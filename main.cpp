#include "WProgram.h"
#include "usb_dev.h"

uint8_t keys[6] = {0,0,0,0,0,0};
void send();

extern "C" int main(void)
{
  pinMode(13, OUTPUT);
  int i = 0;
  while (1) {
    if (i == 5) { keys[0] = 16388; send(); }
    if (i == 6) { keys[0] = 0; send(); }
    i++;
    digitalWriteFast(13, HIGH);
    delay(500);
    digitalWriteFast(13, LOW);
    delay(500);
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
  uint8_t empty = 0;
  *(tx_packet->buf) = empty; // modifiers
  *(tx_packet->buf + 1) = empty; // media;
  memcpy(tx_packet->buf + 2, keys, 6);
  tx_packet->len = 8;
  usb_tx(KEYBOARD_ENDPOINT, tx_packet);
}
