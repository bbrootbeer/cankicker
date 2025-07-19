#include <FlexCAN_T4.h>
FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> Can0;

void canTestCrackCRC8(const CAN_message_t &msg);

void setup(void) {
  Serial.begin(115200); delay(400);
  Can0.begin();
  Can0.setBaudRate(500000);
  Can0.setMaxMB(32); // when using fifo this value doesn't apply to receive because RX is handled via dedicated fifo queue, not mailboxes...
  Can0.enableMBInterrupts(); // enables all mailboxes to be interrupt enabled
}

void loop() {
  // Can0.events();
}


// CRC8 function, polynomial 0x07
uint8_t crc8(const uint8_t *data, uint8_t len) {
    uint8_t crc = 0x00;
    for (uint8_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80)
                crc = (crc << 1) ^ 0x07;
            else
                crc <<= 1;
        }
    }
    return crc;
}

void canTestCrackCRC8(const CAN_message_t &msg) {
    uint8_t buffer[16];
    uint8_t index = 0;

    buffer[index++] = 0xAA; // SOF

    buffer[index++] = (msg.flags.extended ? 0x01 : 0x00);
    buffer[index++] = (msg.id >> 24) & 0xFF;
    buffer[index++] = (msg.id >> 16) & 0xFF;
    buffer[index++] = (msg.id >> 8) & 0xFF;
    buffer[index++] = msg.id & 0xFF;
    buffer[index++] = msg.len;

    for (uint8_t i = 0; i < msg.len; i++) {
        buffer[index++] = msg.buf[i];
    }

    while (index < 15) buffer[index++] = 0x00;

    // Compute CRC8 over everything except SOF (start at index 1)
    uint8_t checksum = crc8(&buffer[1], index - 1);

    buffer[index++] = checksum;

    // Debug print
    for (uint8_t i = 0; i < index; i++) {
        Serial.print(buffer[i], HEX);
        Serial.print(' ');
    }
    Serial.println();

    Serial.write(buffer, index);
}
