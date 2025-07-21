#include <FlexCAN_T4.h>
#include <stdint.h>
#include <stddef.h>
#include <crc16_ccitt_table.h>
FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> Can0;

extern const uint16_t crc16_table[256];

uint16_t crc16_ccitt_table(const uint8_t* data, size_t len, uint16_t init = 0xFFFF) {
    uint16_t crc = init;
    for (size_t i = 0; i < len; ++i) {
        uint8_t tbl_idx = (crc >> 8) ^ data[i]; // high byte XOR input
        crc = (crc << 8) ^ crc16_table[tbl_idx];
    }
    return crc;
}

void sevenOneSeven(const CAN_message_t &msg);

uint16_t crc16_ccitt(const uint8_t* data, size_t len, uint16_t poly = 0x1021, uint16_t init = 0xFFFF) {
    uint16_t crc = init;
    for (size_t i = 0; i < len; ++i) {
        crc ^= (uint16_t)data[i] << 8;
        for (uint8_t j = 0; j < 8; ++j) {
            if (crc & 0x8000) // 
                crc = (crc << 1) ^ poly;
            else
                crc <<= 1;
        }
    }
    return crc;
}

void setup(void) {
  Serial.begin(115200); delay(400);
//   pinMode(6, OUTPUT); digitalWrite(6, LOW); /* optional tranceiver enable pin */
  Can0.begin();
  Can0.setBaudRate(500000);
  Can0.setMaxMB(32);
  Can0.onReceive(sevenOneSeven);

  Can0.enableMBInterrupts(); // enables all mailboxes to be interrupt enabled

}

void loop() {
  // Can0.events();
}

void sevenOneSeven(const CAN_message_t &msg)
{
    uint8_t buffer[64] = {0};  // Zero-initialize entire buffer (safe defaults)
    // uint8_t index = 0;

    buffer[0] = 0xAA;
    buffer[1] = 0x69;
    
    uint32_t can_id = msg.id;
    if (msg.flags.extended) {
        can_id |= 0x80000000;  // EFF flag
    }
    buffer[2] = can_id & 0xFF;
    buffer[3] = (can_id >> 8) & 0xFF;
    buffer[4] = (can_id >> 16) & 0xFF;
    buffer[5] = (can_id >> 24) & 0xFF;

    buffer[6] = msg.len;
    
    for (uint8_t i = 0; i < 8; i++) { // CAN_MAX_DLEN is 8
        if (i < msg.len) {
            buffer[7 + i] = msg.buf[i]; // Copy actual data bytes
        } else {
            buffer[7 + i] = 0x00; // Pad unused data bytes with zeros
        }
    }


    uint16_t crc = crc16_ccitt_table(buffer + 1, 14);

    buffer[15] = (crc >> 8) & 0xFF;
    buffer[16] = crc & 0xFF;
    

    // buffer[62] = (crc >> 8) & 0xFF;
    // buffer[63] = crc & 0xFF;

    // checksum test
    // buffer[63] = checksum ^ 0xFF;  // corrupt the checksum
    // buffer[63] = 0xF1;  // corrupt the checksum
    
    // real checksum
    // buffer[63] = checksum;

    // Serial.write(buffer, 64);
    
    Serial.write(buffer, 17); // Send only the first 17 bytes (0xAA, 0x69, ID, len, data, crc)

    // Serial.print("Buffer: ");
    // for (uint8_t i = 0; i < 64; i++) {
    //     if (buffer[i] < 0x10) Serial.print("0");
    //     Serial.print(buffer[i], HEX);
    //     Serial.print(" ");
    // }
    // Serial.println();
}
