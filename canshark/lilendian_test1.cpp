#include <FlexCAN_T4.h>
FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> Can0;

void sendSocketCANFrame(const CAN_message_t &msg);

void setup(void) {
  Serial.begin(115200); delay(400);
//   pinMode(6, OUTPUT); digitalWrite(6, LOW); /* optional tranceiver enable pin */
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
    // uint8_t this is a 16 byte array, each element being ```uint8_t``` (an 8-bit unsigned byte)
    // {0} syntax initializes all 16 bytes to zero
    // so this is ```frame [16]``` -> 16 elements and each is ```uint8_t``` -> 16bytes total
    // a uint8 is ```0000 0000```
    uint8_t frame[16] = {0};
    
    // Pack can_id (4 bytes, little endian)
    // The FlexCAN_T4 msg.id is usually a 29-bit or 11-bit CAN ID
    // The highest bit (0x80000000) marks extended frame (EFF)
    /// the can_id is stored in a 32-bit integer
    uint32_t can_id = msg.id;
    if (msg.flags.extended) {
        can_id |= 0x80000000;  // EFF flag
    }
    // explaing this can_id & 0xFF
    // frame[0] = bits  0–7   (LSB)
    // frame[1] = bits  8–15
    // frame[2] = bits 16–23
    // frame[3] = bits 24–31 (MSB)
    // lets say that the uint32_t is a extended frame:
    // 1001 1111 | 1111 1111 | 1111 1111 | 1111 1111
    // frame[0] grabs the the lsb          ^
    // frame[0] is 1111 1111
    // so what is 0xFF?? When you do: ```someValue & 0xFF`` it's telling the cpu, to keep the lowest 8 bits, and zero out everything else
    frame[0] = can_id & 0xFF;
    // frame[1] is using bitshifts
    // example of bitshifts, and 0xFF
    // so below is the original uint32_t
    // 1001 1111 | 1111 1111 | 1111 1111 | 1111 1111
    // and with the bitshift >> 8
    // 0000 0000 | 1001 1111 | 1111 1111 | 1111 1111
    // & 0xFF
    // 0000 0000 | 0000 0000 | 0000 0000 | 1111 1111
    // and thats where frame[1] grabs the byte ```1111 1111``` for its uint8_t value
    frame[1] = (can_id >> 8) & 0xFF;
    // so below is the original uint32_t
    // 1001 1111 | 1111 1111 | 1111 1111 | 1111 1111
    // and with the bitshift >> 16
    // 0000 0000 | 0000 0000 | 1001 1111 | 1111 1111
    // & 0xFF
    // 0000 0000 | 0000 0000 | 0000 0000 | 1111 1111
    // and thats where frame[2] grabs the byte ```1111 1111``` for its uint8_t value
    frame[2] = (can_id >> 16) & 0xFF;
        // so below is the original uint32_t
    // 1001 1111 | 1111 1111 | 1111 1111 | 1111 1111
    // and with the bitshift >> 24
    // 0000 0000 | 0000 0000 | 0000 0000 | 1001 1111
    // & 0xFF
    // 0000 0000 | 0000 0000 | 0000 0000 | 1001 1111
    // and thats where frame[3] grabs the byte ```1001 1111``` for its uint8_t value
    frame[3] = (can_id >> 24) & 0xFF;

    // Data length code
    // msg.len is one byte so... yeah
    frame[4] = msg.len; 

    // Next 3 bytes are padding / reserved, leave as 0 (frame[5..7])

    // Data bytes (max 8 bytes)
    // the loop
    // uint8_t i = 0; this is just a counter and has nothing to do with framing, but unlike python...
    // the counter needs to have a declated type...
    // this means "i" is a unsigned 8-bit integer "0000 0000" but it's just a counter not buff or frame...
    // why use uint8_t instead of int? it's the smallest integer type... and avoid negative values (since its unsigned)
    // msg.buf[i] is also a uint8_t
    // okay so msg.len is a uint8_t represents the data length code, of the can frame...
    // the range of msg.len is 0 - 8
    // so yeah basically i starts at 0 and counts up to whatever value msg.len

    // sigh final explanation
    // ```uint8_t i = 0;``` initialize loop counter "i" as "0" an 8-bit unsigned integer (0-255)
    // ```i < msg.len && i < 8;``` means run while "i is less than msg.len" and less than "8" CAN data max  8 bytes
    // ```i++``` after each loop iteration, increment by 1
    // frame[8 + i] = msg.buf[i]; FILL THE FRAME STARTING FROM INDEX 8 WITH EACH BYTE FROM '''msg.buf```
    // so...
    // ```frame[8]``` = ```frame[8 + 0] = msg.buf[0]```
    // ```frame[9]``` = ```frame[8 + 1] = msg.buf[1]```
    // ```frame[14]``` = ```frame[8 + 6] = msg.buf[6]```
    // ```frame[15]``` = ```frame[8 + 7] = msg.buf[7]```


    for (uint8_t i = 0; i < msg.len && i < 8; i++) {
        frame[8 + i] = msg.buf[i];
    }

    // Send the raw 16-byte frame over serial

    // this takes a pointer to the "data buffer" ```frame``` and a length of 16 bytes ```16```
    Serial.write(frame, 16);
}






// 
// struct can_frame {
//     canid_t can_id;  // 4 bytes (with flags!)
//     __u8    can_dlc; // 1 byte
//     __u8    __pad;   // 1 byte
//     __u8    __res0;  // 1 byte
//     __u8    __res1;  // 1 byte
//     __u8    data[8]; // up to 8 bytes
// };


// initialization

// 0000 0000 | 0000 0000 | 0000 0000 | 0000 0000 | 0000 0000 | 0000 0000 | 0000 0000 | 0000 0000 | 0000 0000 | 0000 0000 | 0000 0000 | 0000 0000 | 0000 0000 | 0000 0000 | 0000 0000 | 0000 0000

// This creates a 32-bit integer

// 0000 0000 | 0000 0000 | 0000 0000 | 0000 0000

// uint32_t can_id = msg.id;

// if the id is regular (11 bit), it will look like this:
// 0000 0111 | 1111 1111
// Hex: 0x7FF
// Decimal: 2047
// and when this value gets assigned to a uint32_t it goes from this
// 0000 0111 | 1111 1111
// to this
// 0000 0000 | 0000 0000 | 0000 0111 | 1111 1111

// if the id is extended (29 bit), it will look like this:
// 0001 1111 | 1111 1111 | 1111 1111 | 1111 1111
// Hex: 0x1FFFFFFF
// Decimal: 536,870,911
// this is already a uint32_t i think... idk its the saame size tho
// 0001 1111 | 1111 1111 | 1111 1111 | 1111 1111
// so it stays the same
// 0001 1111 | 1111 1111 | 1111 1111 | 1111 1111
// BUT this part
// if (msg.flags.extended) {
//     can_id |= 0x80000000;  // EFF flag
// }
// turns this:
// 0001 1111 | 1111 1111 | 1111 1111 | 1111 1111
// into this
// 1001 1111 | 1111 1111 | 1111 1111 | 1111 1111
// because 0x80000000 is actually
// 1000 0000 | 0000 0000 | 0000 0000 | 0000 0000

// 0000 0000 | 0000 0000 | 0000 0000 | 0000 0000

// 0000 0000 | 0000 0000 | 0000 0000 | 0000 0000 | 0000 0000 | 0000 0000 | 0000 0000 | 0000 0000 | 0000 0000 | 0000 0000 | 0000 0000 | 0000 0000 | 0000 0000 | 0000 0000 | 0000 0000 | 0000 0000 |
