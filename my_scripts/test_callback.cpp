#include <FlexCAN_T4.h>

FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> can1;
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> can2;

// what is volatil bool? the var can change at any time? always re-read it from memory?
volatile bool new_msg = false;
// CAN_message_t is a struct holdign the CAN frame data (ID, length, data bytes, flags, ect)
CAN_message_t last_msg;

// handleCAN() interrupt sevice routine (ISR) runs outside the main loop
// WITHOUT VOLATILE the compiler might cache ```new_msg``` in a register, and never see the ISR change it
// The ```&msg``` means it’s passed by reference — instead of making a copy, you get a reference (like a pointer, but safer) to the original data.
// ```const``` means you cannot modify msg inside the function — a promise to not change the original data.
void handleCAN(const CAN_message_t &msg) {
  // the entire struct ```msg``` is compied into ```last_msg```
  last_msg = msg;
  new_msg = true;
}

void setup() {
  Serial.begin(115200);

  can1.setTX(DEF); // TX22
  can1.setRX(DEF); // RX23
  can1.begin();
  can1.enableFIFO();
  can1.enableFIFOInterrupt();  // if you're using ISR
  can1.setFIFOFilter(REJECT_ALL);  // clear default filters
  can1.setFIFOFilter(0, STD, 0);      // accept all STD IDs
  can1.setFIFOFilter(1, EXT, 0);      // accept all EXT IDs
  // can1.setFIFOFilter(0, STD, 0x123);  // only match ID 0x123
  // can1.setFIFOFilter(1, EXT, 0x1ABCDEF0);  // only match this EXT ID
  can1.setBaudRate(500000);
  can1.onReceive(handleCAN);

  can2.setTX(DEF);  // TX on pin 1
  can2.setRX(DEF);  // RX on pin 0
  can2.begin();
  can2.enableFIFO();
  can2.enableFIFOInterrupt();
  can2.setFIFOFilter(REJECT_ALL);
  can2.setFIFOFilter(0, STD);
  can2.setFIFOFilter(1, EXT);
  can2.setBaudRate(500000);
  can2.onReceive(handleCAN);
}

// void loop() {
//   if (new_msg) {
//     new_msg = false;
//     Serial.print(millis()); Serial.print(", ID: 0x");
//     Serial.print(last_msg.id, HEX); Serial.print(", DATA: ");
//     for (int i = 0; i < last_msg.len; i++) {
//       Serial.print(last_msg.buf[i], HEX); Serial.print(" ");
//     }
//     Serial.println();
//   }
// }

void loop() {
  if (new_msg) {
    new_msg = false;
    // Check if extended frame or not
    if (last_msg.flags.extended) {
      // Extended frame (29-bit)
      Serial.print('T');
      Serial.printf("%08X", last_msg.id);
    } else {
      // Standard frame (11-bit)
      Serial.print('t');
      Serial.printf("%03X", last_msg.id);
    }
    Serial.print(last_msg.len);
    for (int i = 0; i < last_msg.len; i++) {
      Serial.printf("%02X", last_msg.buf[i]);
    }
    Serial.println();
  }
}
