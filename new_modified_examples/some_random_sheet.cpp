#include <FlexCAN_T4.h>

FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> can1;

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);  // Wait for serial monitor

  can1.begin();
  can1.setBaudRate(500000);  // Most Chrysler vehicles are 500kbps
  can1.setMaxMB(16);
  can1.enableMBInterrupts();
  can1.onReceive(canSniff);
  Serial.println("CAN1 ready.");
}

void loop() {
  can1.events();
}

// Callback when a message is received
void canSniff(const CAN_message_t &msg) {
  Serial.print("ID: 0x"); Serial.print(msg.id, HEX);
  Serial.print("  Len: "); Serial.print(msg.len);
  Serial.print("  Data: ");
  for (int i = 0; i < msg.len; i++) {
    Serial.print(msg.buf[i], HEX); Serial.print(" ");
  }
  Serial.println();
}
