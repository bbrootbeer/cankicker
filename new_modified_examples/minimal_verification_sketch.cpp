#include <FlexCAN_T4.h>

FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> Can0;

void setup() {
  Serial.begin(115200);
  while (!Serial);  // Wait for serial monitor to open
  delay(500);        // Give USB serial time to initialize
  // Can0.setMaxMB(16);  // ✅ Good
  // Can0.setMaxMB(32); // apparently this MUST be set before Can0.begin();
  // Can0.setMaxMB(32);
  // Can0.enableFIFO();             // 🧠 Required for >16 mailboxes
  // Can0.enableFIFOInterrupt();    // optional
  // Can0.setMaxMB(16);         // ✅ Before begin
  Can0.setMaxMB(64);              // 💥 Some Teensy boards need FIFO for this
  Can0.enableFIFO();              // 🧠 Required
  Can0.enableFIFOInterrupt();     // Optional
  Can0.begin();
  Can0.setBaudRate(500000);
  Can0.mailboxStatus();           // ❓ MAY STILL FAIL due to uninitialized mb_configs
  

  Can0.begin();
  Can0.setBaudRate(500000);

  Serial.println("Calling mailboxStatus...");
  Can0.mailboxStatus();  // Should print something!
}

void loop() {
  // Nothing
}
