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

// CRC16 CCITT function, polynomial 0x1021
uint16_t crc16_ccitt(const uint8_t *data, uint8_t len) {
    uint16_t crc = 0xFFFF; // Initial value
    for (uint8_t i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;
            else
                crc <<= 1;
        }
    }
    return crc;
}

void canTestCrackCRC16(const CAN_message_t &msg) {
    uint8_t buffer[18]; // 16 + 2 for CRC16
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

    uint16_t checksum = crc16_ccitt(&buffer[1], index - 1);

    // Append CRC16 high and low bytes
    buffer[index++] = (checksum >> 8) & 0xFF;
    buffer[index++] = checksum & 0xFF;

    // Debug print
    for (uint8_t i = 0; i < index; i++) {
        Serial.print(buffer[i], HEX);
        Serial.print(' ');
    }
    Serial.println();

    Serial.write(buffer, index);
}
