#include <FlexCAN_T4.h>

FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> can1;
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> can2;

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);

  can1.begin();
  can1.setBaudRate(500000);
  can1.setMaxMB(16); // Max mailboxes (optional, defaults to 16)

  for (int i = 0; i < 8; i++) {
    can1.setMB((FLEXCAN_MAILBOX)i, RX, STD); // Or EXT if needed
    can1.setMBFilter((FLEXCAN_MAILBOX)i, 0); // Accept all
    }

  for (int i = 8; i < 16; i++) {
    can1.setMB((FLEXCAN_MAILBOX)i, RX, EXT);          // Mailboxes 8-15 for Extended IDs
    can1.setMBFilter((FLEXCAN_MAILBOX)i, 0);          // Accept all extended IDs
    }

//   can2.begin();
//   can2.setBaudRate(500000);
//   can2.setMaxMB(16);
//   for (int i = 0; i < 16; i++) {
//     can2.setMB((FLEXCAN_MAILBOX)i, RX, STD);
//     can2.setMBFilter((FLEXCAN_MAILBOX)i, 0);
//   }
}


void printSocketCAN(const CAN_message_t& msg) {
  // Format: t<ID><len><data> — standard ID (11-bit), lowercase 't'
  //         T<ID><len><data> — extended ID (29-bit), uppercase 'T'

  if (msg.flags.extended) {
    Serial.print('T');
    Serial.print(msg.id, HEX); // Should be 8 hex chars
  } else {
    Serial.print('t');
    Serial.print(msg.id, HEX); // 3 hex chars
  }

  Serial.print(msg.len, HEX); // One digit (0-F)

  for (int i = 0; i < msg.len; i++) {
    if (msg.buf[i] < 0x10) Serial.print('0');
    Serial.print(msg.buf[i], HEX);
  }

  Serial.print('\r');
}

void loop() {
  CAN_message_t msg;
  while (can1.read(msg)) {
    printSocketCAN(msg);
  }
}