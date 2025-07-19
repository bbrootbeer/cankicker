#include <FlexCAN_T4.h>

FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> Can0;

void setup() {
  Serial.begin(115200);
  while (!Serial); delay(200);

  Serial.println("✅ Setup started");

  Can0.setMaxMB(16);         // ✅ Before begin
  Can0.disableFIFO();        // ✅ Explicitly say no FIFO
  Can0.begin();
  Can0.setBaudRate(500000);

  Serial.println("📦 Calling mailboxStatus...");
  Can0.mailboxStatus();
  Serial.println("✅ mailboxStatus done");
}

void loop() {}
