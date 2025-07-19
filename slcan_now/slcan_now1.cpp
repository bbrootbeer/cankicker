#include <FlexCAN_T4.h>
FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> Can1;

void slcand1(const CAN_message_t &msg);

void setup(void) {
  Serial.begin(115200); delay(500);
  Can1.begin();
  Can1.setBaudRate(500000);
  Can1.setMaxMB(16);
  // TRY THIS HERE NEXT TIME //
  // Can1.enableMBInterrupts();
  // TRY THIS HERE NEXT TIME //
  Can1.onReceive(slcand1);
  Can1.enableMBInterrupts(); // enables all mailboxes to be interrupt enabled
}

void loop() {
// Can1.events();
}

void slcand1(const CAN_message_t &msg) {
  // Determine frame type character based on whether it's extended
  char frameType = msg.flags.extended ? 'T' : 't';
  Serial.print(frameType);

  // Print ID in correct width (3 digits for standard, 8 for extended)
  if (msg.flags.extended) {
    Serial.printf("%08lX", msg.id); // %lX for long hexadecimal
  } else {
    Serial.printf("%03lX", msg.id); // %lX for long hexadecimal
  }

  // Add the data length delimiter '#'
  Serial.print('#');

  // Print data bytes (each 2 hex digits)
  for (uint8_t i = 0; i < msg.len; i++) {
    Serial.printf("%02X", msg.buf[i]);
  }

  // End of SLCAN frame (important)
  Serial.print('\r');
}
