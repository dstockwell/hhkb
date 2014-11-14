#include "WProgram.h"
#include "usb_dev.h"

uint8_t keys[KEYBOARD_SIZE] = {0};
void send();

extern "C" int main(void)
{
  pinMode(13, OUTPUT);
  int i = 0;
  while (1) {
    if (i == 3) { keys[1] = 16; send(); }
    if (i == 4) { keys[1] = 0; send(); }
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
  memcpy(tx_packet->buf, keys, KEYBOARD_SIZE);
  tx_packet->len = KEYBOARD_SIZE;
  usb_tx(KEYBOARD_ENDPOINT, tx_packet);
}
