#include <FlexCAN_T4.h>
FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> Can0;

void sendSocketCANFrame(const CAN_message_t &msg);

void setup(void) {
  Serial.begin(115200); delay(400);
  // pinMode(6, OUTPUT); digitalWrite(6, LOW); /* optional tranceiver enable pin */
  Can0.begin();
  Can0.setBaudRate(500000);
  Can0.setMaxMB(32); // when using fifo this value doesn't apply to receive because RX is handled via dedicated fifo queue, not mailboxes...
  // Can0.enableFIFO(); // required for using the fifo receive queue
  // Can0.enableFIFOInterrupt(); // required for using the fifo receive queue
  Can0.onReceive(sendSocketCANFrame);
  Can0.enableMBInterrupts(); // enables all mailboxes to be interrupt enabled
  // Can0.mailboxStatus();
}

void loop() {
  // Can0.events();
}

void sendSocketCANFrame(const CAN_message_t &msg) {
    // Prepare buffer to match Linux SocketCAN can_frame struct (16 bytes)
    uint8_t frame[16] = {0};
    
    // Pack can_id (4 bytes, little endian)
    // The FlexCAN_T4 msg.id is usually a 29-bit or 11-bit CAN ID
    uint32_t can_id = msg.id;
    if (msg.flags.extended) {
        can_id |= 0x80000000;  // EFF flag
    }
    frame[0] = can_id & 0xFF;
    frame[1] = (can_id >> 8) & 0xFF;
    frame[2] = (can_id >> 16) & 0xFF;
    frame[3] = (can_id >> 24) & 0xFF;

    // Data length code
    frame[4] = msg.len; 

    // Next 3 bytes are padding / reserved, leave as 0 (frame[5..7])

    // Data bytes (max 8 bytes)
    for (uint8_t i = 0; i < msg.len && i < 8; i++) {
        frame[8 + i] = msg.buf[i];
    }

    // Send the raw 16-byte frame over serial
    Serial.write(frame, 16);
    // Serial.println(); this will never work for what you want it for...
}
