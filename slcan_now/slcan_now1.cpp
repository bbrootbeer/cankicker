#include <FlexCAN_T4.h>
FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> Can1;

void slcand1(const CAN_message_t &msg);
void canss(const CAN_message_t &msg);

void setup(void) {
  Serial.begin(115200); delay(500);
  Can1.begin();
  Can1.setBaudRate(500000);
  Can1.setMaxMB(16);
  // TRY THIS HERE NEXT TIME //
  // Can1.enableMBInterrupts();
  // TRY THIS HERE NEXT TIME //
  Can1.onReceive(slcand1);
  Can1.onReceive(canss);
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

void canss(const CAN_message_t &msg) {
  Serial.print("MB "); Serial.print(msg.mb);
  Serial.print("  OVERRUN: "); Serial.print(msg.flags.overrun);
  Serial.print("  LEN: "); Serial.print(msg.len);
  Serial.print(" EXT: "); Serial.print(msg.flags.extended);
  Serial.print(" TS: "); Serial.print(msg.timestamp);
  Serial.print(" ID: "); Serial.print(msg.id, HEX);
  // MB 0  OVERRUN: 0  LEN: 5 EXT: 0 TS: 42138 ID: 262 Buffer: 0 0 0 0 69
  // MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 42303 ID: 2C4 Buffer: 3 87 0 30 0 80 61 69
  // MB 0  OVERRUN: 0  LEN: 3 EXT: 0 TS: 44802 ID: 20 Buffer: 0 0 7 
  // Serial.print(" ID: "); Serial.print(msg.id);
  // MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 15629 ID: 1552 Buffer: 20 0 0 64 0 0 0 0
  // MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 18107 ID: 708 Buffer: 3 8D 0 30 0 80 61 6F
  // MB 0  OVERRUN: 0  LEN: 3 EXT: 0 TS: 18246 ID: 32 Buffer: 0 0 7
  Serial.print(" Buffer: ");
  for ( uint8_t i = 0; i < msg.len; i++ ) {
    Serial.print(msg.buf[i], HEX); Serial.print(" ");
  } Serial.println();
}