#include "usb_dev.h"

class RawHid {
public:
  uint8_t tx[RAWHID_TX_SIZE] = {0};
  uint8_t rx[RAWHID_RX_SIZE] = {0};

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
    memcpy(tx_packet->buf, tx, RAWHID_TX_SIZE);
    tx_packet->len = RAWHID_TX_SIZE;
    usb_tx(RAWHID_TX_ENDPOINT, tx_packet);
  }

  int available() {
    if (!usb_configuration) {
      return 0;
    }
    return usb_rx_byte_count(RAWHID_RX_ENDPOINT);
  }

  int receive() {
    usb_packet_t *rx_packet;

    while (1) {
      if (!usb_configuration) return -1;
      rx_packet = usb_rx(RAWHID_RX_ENDPOINT);
      if (rx_packet) break;
    }
    uint32_t len = min(RAWHID_RX_SIZE, rx_packet->len);
    memcpy(rx, rx_packet->buf, min(RAWHID_RX_SIZE, rx_packet->len));
    usb_free(rx_packet);
    return len;
  }
};

