#include <FlexCAN_T4.h>
FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> Can0;

void sevenOneSeven(const CAN_message_t &msg);

void setup(void) {
  Serial.begin(115200); delay(400);
//   pinMode(6, OUTPUT); digitalWrite(6, LOW); /* optional tranceiver enable pin */
  Can0.begin();
  Can0.setBaudRate(500000);
  Can0.setMaxMB(32); // when using fifo this value doesn't apply to receive because RX is handled via dedicated fifo queue, not mailboxes...
  // Can0.enableFIFO(); // required for using the fifo receive queue
  // Can0.enableFIFOInterrupt(); // required for using the fifo receive queue
  Can0.onReceive(sevenOneSeven);
  // Can0.onReceive(canKick);
  // Can0.onReceive(canCrack);
//   Can0.onReceive(canTestCrack);
  Can0.enableMBInterrupts(); // enables all mailboxes to be interrupt enabled
  
  // Can0.mailboxStatus();
}

void loop() {
  // Can0.events();
}

void sevenOneSeven(const CAN_message_t &msg)
{
    uint8_t buffer[64] = {0};  // Zero-initialize entire buffer (safe defaults)
    // uint8_t index = 0;

    buffer[0] = 0x69; // index 0
    buffer[1] = 0xAA;
    
    uint8_t checksum = 0x00;
    for (uint8_t i = 0; i < 63; i++) {
        checksum ^= buffer[i];
    }
    buffer[63] = checksum;

    Serial.print("Buffer: ");
    for (uint8_t i = 0; i < 64; i++) {
        if (buffer[i] < 0x10) Serial.print("0");
        Serial.print(buffer[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}

// Buffer: 69 AA 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 C3 
// Buffer: 69 AA 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 C3 
// Buffer: 69 AA 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 C3
