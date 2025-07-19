#include <FlexCAN_T4.h>

FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> Can0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("=== Starting Mailbox Test ===");

  Can0.begin();
  Can0.setBaudRate(500000);
  Can0.setMaxMB(16); // try with 16 or 64
  // FIFO disabled on purpose for clarity
  // Can0.enableFIFO();
  // Can0.enableFIFOInterrupt();

  Serial.println("Calling mailboxStatus()...");
  Can0.mailboxStatus();
  Serial.println("=== Done ===");
}

void loop() {}
