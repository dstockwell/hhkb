// See: https://github.com/tmk/tmk_keyboard/blob/master/keyboard/hhkb/doc/HHKB.txt

class Hhkb {
public:
  uint64_t matrix() const { return m_matrix; }
  uint64_t prevMatrix() const { return m_prevMatrix; }
  bool scan() {
    m_prevMatrix = m_matrix;
    m_matrix = 0;
    for (uint8_t row = 0; row < 8; row++) {
      for (uint8_t col = 0; col < 8; col++) {
        select(row, col);
        delayMicroseconds(15);
        enable();
        delayMicroseconds(5);
  
        if (read()) {
          m_matrix |= UINT64_C(1) << (row * 8 + col);
        }
  
        delayMicroseconds(5);
        disable();
        // FIXME: This should delay the next scan, not the return.
        delayMicroseconds(75);
      }
    }
    return m_matrix != m_prevMatrix;
  }

  void init() {
    pinMode(0, INPUT_PULLUP);
    for (uint8_t i = 1; i < 9; i++) {
      pinMode(i, OUTPUT);
    }
  }

private:
  uint64_t m_matrix;
  uint64_t m_prevMatrix;

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
};
