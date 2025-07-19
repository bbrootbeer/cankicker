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

uint32_t crc32(const uint8_t *data, size_t length) {
    uint32_t crc = 0xFFFFFFFF;

    for (size_t i = 0; i < length; i++) {
        uint8_t byte = data[i];
        crc ^= byte;

        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;
            else
                crc >>= 1;
        }
    }

    return ~crc;
}

void sendSocketCANFrame(const CAN_message_t &msg) {
    
    // Prepare buffer to match Linux SocketCAN can_frame struct (16 bytes)
    uint8_t frame[25] = {0};
    
    // SOF
    frame[0] = 0x69;
    
    
    uint32_t timestamp = micros();
    frame[1] = timestamp & 0xFF;
    frame[2] = (timestamp >> 8) & 0xFF;
    frame[3] = (timestamp >> 16) & 0xFF;
    frame[4] = (timestamp >> 24) & 0xFF;

    // Pack can_id (4 bytes, little endian)
    // The FlexCAN_T4 msg.id is usually a 29-bit or 11-bit CAN ID
    uint32_t can_id = msg.id;
    if (msg.flags.extended) {
        can_id |= 0x80000000;  // EFF flag
    }
    frame[5] = can_id & 0xFF;
    frame[6] = (can_id >> 8) & 0xFF;
    frame[7] = (can_id >> 16) & 0xFF;
    frame[8] = (can_id >> 24) & 0xFF;

    // Data length code
    frame[9] = msg.len; 

    // // Next 3 bytes are padding / reserved, leave as 0 (frame[5..7])
    // Next 3 bytes are padding / reserved, leave as 0 (frame[10..12])


    // Data bytes (max 8 bytes)
    for (uint8_t i = 0; i < msg.len && i < 8; i++) {
        frame[13 + i] = msg.buf[i];
    }

    // CRC32 crc;
    // crc.update(&frame[1], 20);
    // uint32_t crcVal = crc.finalize();

    uint32_t crcVal = crc32(&frame[1], 20);

    frame[21] = crcVal & 0xFF;
    frame[22] = (crcVal >> 8) & 0xFF;
    frame[23] = (crcVal >> 16) & 0xFF;
    frame[24] = (crcVal >> 24) & 0xFF;

    // Send the raw 25-byte frame over serial
    Serial.write(frame, 25);
    Serial.println(); // 👈 This gives you a line break per frame
    // Serial.write('\n');

}

// pio device monitor -b 115200 --encoding hexlify
// pio device monitor -b 115200 --echo --encoding hexlify --eol LF


// struct can_frame {
//     canid_t can_id;  // 4 bytes
//     __u8    can_dlc; // 1 byte
//     __u8    __pad;   // 1 byte (reserved)
//     __u8    __res0;  // 1 byte
//     __u8    __res1;  // 1 byte
//     __u8    data[8]; // 8 bytes
// };