#include <FlexCAN_T4.h>
#include <stdint.h>
#include <stddef.h>
#include <crc16_ccitt_table.h>
FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> Can0;

// This should match your generated table what is this? for the table
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

// The CRC polynomial like 0x1021 represents a math expression used in polynomial division. But in binary, that’s all it is — a bitmask. So:

// 0x1021 = 0001 0000 0010 0001
// (That's 17 bits if we write it fully!)

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



//        
//        0b0001000000100001 = 0x1021
//        0b1000000000000000 = 0x8000
// crc1 = 0b1111111111111111
// crc2 = 0b1010101000000000
// crc3 = 0b0101010111111111
//     if 0b0101010111111111 & 0b1000000000000000 is true? (it's false i guess) and no this doesn't modify it
//         0b1010101111111110 this time it's true so shift it
//          0b0101011111111100 and square the polynomial
//          0b0001000000100001 = 0x1021
//          0b0100011111011101, it's false so just shift it
//           0b1000111110111010 its true so... shift it
//            0b0001111101110100
//            0b0101011111111100
//            0b0100100010001000 its false
//             0b1001000100010000 its true
//              0b0010001000100000
//              0b0001000000100001
//              0b0011001000000001 its false
//               0b0110010000000010 its false
//                0b1100100000000100 final return
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
  

    // CRC calculation: only use first 62 bytes
    // uint16_t crc = crc16_ccitt(buffer, 62);
    // uint16_t crc = crc16_ccitt(buffer + 1, 1);
    uint16_t crc = crc16_ccitt_table(buffer + 1, 1);

    buffer[2] = (crc >> 8) & 0xFF;
    buffer[3] = crc & 0xFF;

    // buffer[62] = (crc >> 8) & 0xFF;
    // buffer[63] = crc & 0xFF;

    // checksum test
    // buffer[63] = checksum ^ 0xFF;  // corrupt the checksum
    // buffer[63] = 0xF1;  // corrupt the checksum
    
    // real checksum
    // buffer[63] = checksum;

    // Serial.write(buffer, 64);

    Serial.print("Buffer: ");
    for (uint8_t i = 0; i < 64; i++) {
        if (buffer[i] < 0x10) Serial.print("0");
        Serial.print(buffer[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}

// Buffer: 69 AA 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 AA 
// Buffer: 69 AA 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 AA 
// Buffer: 69 AA 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 AA 

// uint16_t crc = crc16_ccitt(buffer + 1, 1);
// Buffer: 69 AA F5 50 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
// Buffer: 69 AA F5 50 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
// Buffer: 69 AA F5 50 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 

// uint16_t crc = crc16_ccitt_table(buffer + 1, 1);
// Buffer: 69 AA F5 50 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
// Buffer: 69 AA F5 50 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
// Buffer: 69 AA F5 50 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 