#include <FlexCAN_T4.h>
FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> Can0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Starting...");

  Can0.begin();
  Can0.setBaudRate(500000);
  Can0.setMaxMB(16);

  // No FIFO
  // Can0.enableFIFO();  <--- comment this out
  // Can0.enableFIFOInterrupt();

  Can0.mailboxStatus();  // Should now print something
}

void loop() {}
