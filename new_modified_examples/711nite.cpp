#include <FlexCAN_T4.h>
FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> Can0;

void canSniff(const CAN_message_t &msg);
void canKick(const CAN_message_t &msg);
void canCrack(const CAN_message_t &msg);
void canCrack2(const CAN_message_t &msg);
void canTestCrack(const CAN_message_t &msg);



void setup(void) {
  Serial.begin(115200); delay(400);
//   pinMode(6, OUTPUT); digitalWrite(6, LOW); /* optional tranceiver enable pin */
  Can0.begin();
  Can0.setBaudRate(500000);
  Can0.setMaxMB(32); // when using fifo this value doesn't apply to receive because RX is handled via dedicated fifo queue, not mailboxes...
  // Can0.enableFIFO(); // required for using the fifo receive queue
  // Can0.enableFIFOInterrupt(); // required for using the fifo receive queue
  Can0.onReceive(canSniff);
  // Can0.onReceive(canKick);
  // Can0.onReceive(canCrack);
//   Can0.onReceive(canTestCrack);
  Can0.enableMBInterrupts(); // enables all mailboxes to be interrupt enabled
  
  // Can0.mailboxStatus();
}

void canSniff(const CAN_message_t &msg) {
  Serial.print("MB "); Serial.print(msg.mb);
  Serial.print("  OVERRUN: "); Serial.print(msg.flags.overrun);
  Serial.print("  LEN: "); Serial.print(msg.len);
  Serial.print(" EXT: "); Serial.print(msg.flags.extended);
  Serial.print(" TS: "); Serial.print(msg.timestamp);
  Serial.print(" ID: "); Serial.print(msg.id, HEX);
  // MB 0  OVERRUN: 0  LEN: 5 EXT: 0 TS: 42138 ID: 262 Buffer: 0 0 0 0 69
  // MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 42303 ID: 2C4 Buffer: 3 87 0 30 0 80 61 69
  // MB 0  OVERRUN: 0  LEN: 3 EXT: 0 TS: 44802 ID: 20 Buffer: 0 0 7 
  // Serial.print(" ID: "); Serial.print(msg.id);
  // MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 15629 ID: 1552 Buffer: 20 0 0 64 0 0 0 0
  // MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 18107 ID: 708 Buffer: 3 8D 0 30 0 80 61 6F
  // MB 0  OVERRUN: 0  LEN: 3 EXT: 0 TS: 18246 ID: 32 Buffer: 0 0 7
  Serial.print(" Buffer: ");
  for ( uint8_t i = 0; i < msg.len; i++ ) {
    Serial.print(msg.buf[i], HEX); Serial.print(" ");
  } Serial.println();
}

void loop() {
  // Can0.events();

//   static uint32_t timeout = millis();
//   if ( millis() - timeout > 200 ) {
//     CAN_message_t msg;
//     msg.id = random(0x1,0x7FE);
//     for ( uint8_t i = 0; i < 8; i++ ) msg.buf[i] = i + 1;
//     Can0.write(msg);
//     timeout = millis();
//   }

}

void canKick(const CAN_message_t &msg)
{
    // decide on fram type character based on whether it's extended
    char frameType = msg.flags.extended ? 'T' : 't';
    Serial.print(frameType);

    // Print ID in correct width
    if (msg.flags.extended)
    {
        Serial.printf("%08X", msg.id);
    }
    else
    {
        Serial.printf("%03X", msg.id);
    }

    // print length (1 hex digit)
    Serial.printf("%1X", msg.len);

    // Print data bytes (each 2 hex digits)
    for (uint8_t i = 0; i < msg.len; i++)
    {
        Serial.printf("%02X", msg.buf[i]);
    }

    // End of SLCAN frame (imporant)
    Serial.print('\r');
}

// In file included from lib\FlexCAN_T4/FlexCAN_T4.h:561,
//                  from src\..\new_modified_examples\CAN2.0_example_FIFO_with_interrupts.cpp:1,
//                  from src\main.cpp:5:
// lib\FlexCAN_T4/FlexCAN_T4.tpp:31:2: warning: #warning "HEY! Using fork of FlexCAN_T4" [-Wcpp]       
//    31 | #warning "HEY! Using fork of FlexCAN_T4"

void canCrack(const CAN_message_t &msg)
{
    uint8_t buffer[16];
    uint8_t index = 0;

    // Start-of-frame marker
    Serial.write(0x69);  // or 0x55, your choice

    // FLAGS: bit 0 = extended
    buffer[index++] = (msg.flags.extended ? 0x01 : 0x00);

    // ID (always write 4 bytes)
    buffer[index++] = (msg.id >> 24) & 0xFF;
    buffer[index++] = (msg.id >> 16) & 0xFF;
    buffer[index++] = (msg.id >> 8) & 0xFF;
    buffer[index++] = msg.id & 0xFF;

    // Data length
    buffer[index++] = msg.len;

    // Data bytes (0–8)
    for (uint8_t i = 0; i < msg.len; i++)
    {
        buffer[index++] = msg.buf[i];
    }

    // Optional: pad unused bytes with zero (not required but keeps length fixed)
    while (index < 15) buffer[index++] = 0x00;

    // Compute XOR checksum of all bytes in buffer so far (not including SOF)
    uint8_t checksum = 0x00;
    for (uint8_t i = 0; i < index; i++) checksum ^= buffer[i];

    // Append checksum
    buffer[index++] = checksum;

    // Write the whole frame
    Serial.write(buffer, index);
}

void canTestCrack(const CAN_message_t &msg)
{
    uint8_t buffer[16]; // this creates an array of 16 bytes (0-15) // named array variable
    uint8_t index = 0;  // this initializes and index variable to keep track of where you are in the array

    // Start-of-frame marker
    buffer[index++] = 0x69; // this puts 0xAA into the buffer (currently set to index 0) and then [index++] increases it to, index 1, apparently this is "cleaner and safer" than manually tracking buffer positions, this is a single byte

    buffer[index++] = (msg.flags.extended ? 0x01 : 0x00); // if the can frame is extended then store 0x01 otherwise stor 0x00, at index 1 this is a single byte, and increases the index to 2
    buffer[index++] = (msg.id >> 24) & 0xFF; // i think this is the part that takes the message id (32 bit (4 bytes)) and this takes 31 - 24 bit of the message, and stores it in the buff, at index 2, and increases the index to 3
    buffer[index++] = (msg.id >> 16) & 0xFF; // takes bits 23 - 16 and places them into index 3, and increases the index to 4
    buffer[index++] = (msg.id >> 8) & 0xFF; // takes bit 15 - 8 places them into index 4, and increases the index to 5
    buffer[index++] = msg.id & 0xFF; // bits 7 - 0 places them into index 5 and increases the index to 6
    buffer[index++] = msg.len; // msg.len must be a 8 bit value, inserted into index 6 and increases it to index 7

    for (uint8_t i = 0; i < msg.len; i++) {
        buffer[index++] = msg.buf[i];
    }

    while (index < 15) buffer[index++] = 0x00;

    uint8_t checksum = 0x00;
    for (uint8_t i = 1; i < index; i++) checksum ^= buffer[i];  // exclude SOF from checksum

    buffer[index++] = checksum;

    // DEBUG: print each byte in hex to monitor
    // for (uint8_t i = 0; i < index; i++) {
    //     Serial.print(buffer[i], HEX);
    //     Serial.print(' ');
    // }
    // Serial.println();

    // Actual transmission over serial:
    Serial.write(buffer, index);
}

void canTestCrack2(const CAN_message_t &msg)
{
    uint8_t buffer[16] = {0};  // Zero-initialize entire buffer (safe defaults)
    uint8_t index = 0;

    // Start-of-frame marker (custom protocol)
    buffer[index++] = 0x69; // this is called "post-increment"
    // buffer[index++] = 0x69;
    // // is equivalent to:
    // buffer[0] = 0x69;
    // index = 1;

    // Frame type flag: 0x01 = extended, 0x00 = standard
    // this is either 0x01 (0000 0001)
    // or this 0x00 (0000 0000)
    buffer[index++] = msg.flags.extended ? 0x01 : 0x00;

    // CAN ID in big endian order (most significant byte first)
    buffer[index++] = (msg.id >> 24) & 0xFF;
    buffer[index++] = (msg.id >> 16) & 0xFF;
    buffer[index++] = (msg.id >> 8) & 0xFF;
    buffer[index++] = msg.id & 0xFF;

    // Data Length Code (0–8)
    // msg.len is indeed a uint8_t
    // uint8_t len; this means it's an 8-bit unsigned integer, range 0-255
    // valid values are limited based on the CAN spec
    // for classica can, the values 0 to 8 are legal, and valid...
    buffer[index++] = msg.len;

    // Copy data bytes (up to 8), pad rest with 0x00 (already done by initialization)
    // this starts at index 7
    // msg.buf[0] to msg.buf[msg.len - 1]
    // 
    // In most systems, right-padding is the norm. The data is always placed at the start (buf[0]) and padded at the end if necessary.
    for (uint8_t i = 0; i < msg.len && i < 8; i++) {
        buffer[index++] = msg.buf[i];
    }

    // buffer[index++] = msg.buf[0] // position 7
    // buffer[index++] = msg.buf[1]
    // buffer[index++] = msg.buf[2]
    // buffer[index++] = msg.buf[3]
    // buffer[index++] = msg.buf[4]
    // buffer[index++] = msg.buf[5]
    // buffer[index++] = msg.buf[6] // position 13
    // buffer[index++] = msg.buf[7] // position 14



    // Calculate checksum (XOR of all bytes except SOF)
    uint8_t checksum = 0x00;
    for (uint8_t i = 1; i < index; i++) {
        checksum ^= buffer[i];
    }

    // Add checksum as final byte (index should be 15)
    buffer[index++] = checksum;

    // Send 16-byte packet
    Serial.write(buffer, 16);
}

// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 38796 ID: B4 Buffer: 0 0 0 0 88 0 0 44
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 38919 ID: 25 Buffer: 0 E5 0 2 78 78 78 7C
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 39036 ID: 24 Buffer: 2 1 1 F3 62 C 80 11
// MB 0  OVERRUN: 0  LEN: 5 EXT: 0 TS: 39481 ID: 262 Buffer: 0 0 0 0 69
// MB 0  OVERRUN: 0  LEN: 6 EXT: 0 TS: 41733 ID: B0 Buffer: 0 0 0 0 11 6
// MB 0  OVERRUN: 0  LEN: 6 EXT: 0 TS: 41837 ID: B2 Buffer: 0 0 0 0 11 6
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 41942 ID: 224 Buffer: 0 0 0 0 0 0 0 8
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 42779 ID: 3A0 Buffer: 0 A1 0 0 0 0 0 30 
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 44490 ID: 260 Buffer: 8 0 C 0 0 0 0 7E
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 44629 ID: 2C4 Buffer: 3 85 0 2C 0 80 61 63
// MB 0  OVERRUN: 0  LEN: 3 EXT: 0 TS: 44748 ID: 20 Buffer: 0 0 7
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 44826 ID: B4 Buffer: 0 0 0 0 88 0 0 44
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 44949 ID: 25 Buffer: 0 E5 0 2 78 78 78 7C
// MB 0  OVERRUN: 0  LEN: 4 EXT: 0 TS: 45066 ID: BA Buffer: 5 EF 7 B9
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 45149 ID: 24 Buffer: 2 1 1 F3 62 B 80 10
// MB 0  OVERRUN: 0  LEN: 4 EXT: 0 TS: 45619 ID: 361 Buffer: 0 0 0 0
// MB 0  OVERRUN: 0  LEN: 6 EXT: 0 TS: 47754 ID: B0 Buffer: 0 0 0 0 11 7
// MB 0  OVERRUN: 0  LEN: 6 EXT: 0 TS: 47860 ID: B2 Buffer: 0 0 0 0 11 7
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 47965 ID: 223 Buffer: C0 20 0 0 0 0 0 D
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 48732 ID: 2D0 Buffer: 6 FA 9 0 70 0 0 53
// MB 0  OVERRUN: 0  LEN: 5 EXT: 0 TS: 49448 ID: 262 Buffer: 0 0 0 0 69
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 49725 ID: 2C1 Buffer: 8 7 23 FE FF F9 0 F3
// MB 0  OVERRUN: 0  LEN: 3 EXT: 0 TS: 50760 ID: 20 Buffer: 0 0 7 
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 50838 ID: B4 Buffer: 0 0 0 0 88 0 0 44
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 50961 ID: 25 Buffer: 0 E5 0 2 78 78 78 7C
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 51280 ID: 24 Buffer: 2 1 1 F3 62 C 7F 10
// MB 0  OVERRUN: 0  LEN: 6 EXT: 0 TS: 53765 ID: B0 Buffer: 0 0 0 0 11 8
// MB 0  OVERRUN: 0  LEN: 6 EXT: 0 TS: 53868 ID: B2 Buffer: 0 0 0 0 11 8
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 53972 ID: 224 Buffer: 0 0 0 0 0 0 0 8
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 54097 ID: 3B7 Buffer: 0 0 0 0 0 0 0 0
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 54475 ID: 260 Buffer: 8 0 C 0 0 0 0 7E
// MB 0  OVERRUN: 0  LEN: 3 EXT: 0 TS: 56762 ID: 20 Buffer: 0 0 7
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 56840 ID: B4 Buffer: 0 0 0 0 88 0 0 44
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 56963 ID: 24 Buffer: 2 1 1 F3 62 D 80 12
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 57083 ID: 25 Buffer: 0 E5 0 2 78 78 78 7C
// MB 0  OVERRUN: 0  LEN: 4 EXT: 0 TS: 57200 ID: BA Buffer: 5 EF 7 B9
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 57283 ID: 2C4 Buffer: 3 80 0 2C 0 80 61 5E
// MB 0  OVERRUN: 0  LEN: 5 EXT: 0 TS: 59548 ID: 262 Buffer: 0 0 0 0 69 
// MB 0  OVERRUN: 0  LEN: 6 EXT: 0 TS: 59772 ID: B0 Buffer: 0 0 0 0 11 9
// MB 0  OVERRUN: 0  LEN: 6 EXT: 0 TS: 59875 ID: B2 Buffer: 0 0 0 0 11 9
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 59979 ID: 223 Buffer: C0 20 0 0 0 0 0 D
// MB 0  OVERRUN: 0  LEN: 3 EXT: 0 TS: 62776 ID: 20 Buffer: 0 0 7
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 62854 ID: B4 Buffer: 0 0 0 0 88 0 0 44
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 62977 ID: 25 Buffer: 0 E5 0 2 78 78 78 7C
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 63094 ID: 24 Buffer: 2 1 1 F2 62 C 80 10
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 64429 ID: 260 Buffer: 8 0 C 0 0 0 0 7E 
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 65157 ID: 2D0 Buffer: 6 F6 9 0 70 0 0 4F
// MB 0  OVERRUN: 0  LEN: 6 EXT: 0 TS: 253 ID: B0 Buffer: 0 0 0 0 11 A
// MB 0  OVERRUN: 0  LEN: 6 EXT: 0 TS: 357 ID: B2 Buffer: 0 0 0 0 11 A
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 460 ID: 224 Buffer: 0 0 0 0 0 0 0 8
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 623 ID: 2C1 Buffer: 8 7 21 FE FE F9 0 F0
// MB 0  OVERRUN: 0  LEN: 3 EXT: 0 TS: 3252 ID: 20 Buffer: 0 0 7
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 3330 ID: B4 Buffer: 0 0 0 0 88 0 0 44
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 3453 ID: 25 Buffer: 0 E5 0 2 78 78 78 7C
// MB 0  OVERRUN: 0  LEN: 4 EXT: 0 TS: 3570 ID: BA Buffer: 5 EE 7 B8
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 3666 ID: 24 Buffer: 2 1 1 F3 62 C 80 11
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 3787 ID: 2C4 Buffer: 3 7F 0 2C 0 80 61 5D
// MB 0  OVERRUN: 0  LEN: 5 EXT: 0 TS: 3906 ID: 262 Buffer: 0 0 0 0 69
// MB 0  OVERRUN: 0  LEN: 6 EXT: 0 TS: 6272 ID: B0 Buffer: 0 0 0 0 11 B
// MB 0  OVERRUN: 0  LEN: 6 EXT: 0 TS: 6376 ID: B2 Buffer: 0 0 0 0 11 B
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 6479 ID: 223 Buffer: C0 20 0 0 0 0 0 D
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 8894 ID: 260 Buffer: 8 0 C 0 0 0 0 7E
// MB 0  OVERRUN: 0  LEN: 3 EXT: 0 TS: 9272 ID: 20 Buffer: 0 0 7
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 9350 ID: B4 Buffer: 0 0 0 0 88 0 0 44
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 9473 ID: 25 Buffer: 0 E5 0 2 78 78 78 7C
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 9806 ID: 24 Buffer: 2 1 1 F3 62 C 80 11 
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 11630 ID: 442 Buffer: 40 2 0 0 3 0 0 0
// MB 0  OVERRUN: 0  LEN: 6 EXT: 0 TS: 12293 ID: B0 Buffer: 0 0 0 0 11 C
// MB 0  OVERRUN: 0  LEN: 6 EXT: 0 TS: 12397 ID: B2 Buffer: 0 0 0 0 11 C
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 12500 ID: 224 Buffer: 0 0 0 0 0 0 0 8
// MB 0  OVERRUN: 0  LEN: 4 EXT: 0 TS: 12944 ID: 361 Buffer: 0 0 0 0
// MB 0  OVERRUN: 0  LEN: 5 EXT: 0 TS: 13859 ID: 262 Buffer: 0 0 0 0 69
// MB 0  OVERRUN: 0  LEN: 3 EXT: 0 TS: 15296 ID: 20 Buffer: 0 0 7
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 15374 ID: B4 Buffer: 0 0 0 0 88 0 0 44
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 15497 ID: 24 Buffer: 2 1 1 F3 62 C 80 11
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 15618 ID: 25 Buffer: 0 E5 0 2 78 78 78 7C
// MB 0  OVERRUN: 0  LEN: 4 EXT: 0 TS: 15735 ID: BA Buffer: 5 EF 7 B9
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 16055 ID: 2D0 Buffer: 6 F1 9 0 70 0 0 4A
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 16185 ID: 2C4 Buffer: 3 81 0 2C 0 80 61 5F
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 17057 ID: 2C1 Buffer: 8 7 21 FF 2 F9 0 F5
// MB 0  OVERRUN: 0  LEN: 6 EXT: 0 TS: 18314 ID: B0 Buffer: 0 0 0 0 11 D 
// MB 0  OVERRUN: 0  LEN: 6 EXT: 0 TS: 18418 ID: B2 Buffer: 0 0 0 0 11 D
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 18521 ID: 223 Buffer: C0 20 0 0 0 0 0 D
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 18867 ID: 260 Buffer: 8 0 C 0 0 0 0 7E
// MB 0  OVERRUN: 0  LEN: 3 EXT: 0 TS: 21328 ID: 20 Buffer: 0 0 7
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 21406 ID: B4 Buffer: 0 0 0 0 88 0 0 44
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 21529 ID: 25 Buffer: 0 E5 0 2 78 78 78 7C
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 21646 ID: 24 Buffer: 2 1 1 F3 62 C 80 11
// MB 0  OVERRUN: 0  LEN: 5 EXT: 0 TS: 23860 ID: 262 Buffer: 0 0 0 0 69
// MB 0  OVERRUN: 0  LEN: 6 EXT: 0 TS: 24338 ID: B0 Buffer: 0 0 0 0 11 E
// MB 0  OVERRUN: 0  LEN: 6 EXT: 0 TS: 24441 ID: B2 Buffer: 0 0 0 0 11 E
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 24545 ID: 224 Buffer: 0 0 0 0 0 0 0 8
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 27258 ID: 3A0 Buffer: 0 A1 0 0 0 0 0 30 
// MB 0  OVERRUN: 0  LEN: 3 EXT: 0 TS: 27381 ID: 20 Buffer: 0 0 7
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 27459 ID: B4 Buffer: 0 0 0 0 88 0 0 44
// MB 0  OVERRUN: 0  LEN: 4 EXT: 0 TS: 27582 ID: BA Buffer: 5 EF 7 B9
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 27721 ID: 24 Buffer: 2 1 1 F3 62 C 80 11
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 27842 ID: 25 Buffer: 0 E5 0 2 82 82 82 9A
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 28373 ID: 2C4 Buffer: 3 80 0 2C 0 80 61 5E
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 28935 ID: 260 Buffer: 8 0 C 0 0 0 0 7E
// MB 0  OVERRUN: 0  LEN: 6 EXT: 0 TS: 30447 ID: B0 Buffer: 0 0 0 0 11 F
// MB 0  OVERRUN: 0  LEN: 6 EXT: 0 TS: 30550 ID: B2 Buffer: 0 0 0 0 11 F
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 30654 ID: 223 Buffer: C0 20 0 0 0 0 0 D
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 31979 ID: 610 Buffer: 20 0 0 64 0 0 0 0
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 32480 ID: 2D0 Buffer: 6 EE 9 0 70 0 0 47
// MB 0  OVERRUN: 0  LEN: 3 EXT: 0 TS: 33444 ID: 20 Buffer: 0 0 7
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 33522 ID: B4 Buffer: 0 0 0 0 88 0 0 44
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 33645 ID: 2C1 Buffer: 8 7 21 FF 2 F9 0 F5
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 33775 ID: 25 Buffer: 0 E5 0 2 78 78 78 7C
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 33892 ID: 24 Buffer: 2 1 1 F3 62 D 80 12
// MB 0  OVERRUN: 0  LEN: 5 EXT: 0 TS: 34012 ID: 262 Buffer: 0 0 0 0 69
// MB 0  OVERRUN: 0  LEN: 6 EXT: 0 TS: 36458 ID: B0 Buffer: 0 0 0 0 11 0 
// MB 0  OVERRUN: 0  LEN: 6 EXT: 0 TS: 36563 ID: B2 Buffer: 0 0 0 0 11 0
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 36667 ID: 224 Buffer: 0 0 0 0 0 0 0 8
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 38827 ID: 260 Buffer: 8 0 C 0 0 0 0 7E
// MB 0  OVERRUN: 0  LEN: 3 EXT: 0 TS: 39453 ID: 20 Buffer: 0 0 7
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 39531 ID: B4 Buffer: 0 0 0 0 88 0 0 44
// MB 0  OVERRUN: 0  LEN: 4 EXT: 0 TS: 39654 ID: BA Buffer: 5 EE 7 B8
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 39776 ID: 25 Buffer: 0 E5 0 2 78 78 78 7C
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 40009 ID: 24 Buffer: 2 1 1 F3 62 B 80 10
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 40696 ID: 2C4 Buffer: 3 84 0 2C 0 80 61 62
// MB 0  OVERRUN: 0  LEN: 6 EXT: 0 TS: 42468 ID: B0 Buffer: 0 0 0 0 11 1
// MB 0  OVERRUN: 0  LEN: 6 EXT: 0 TS: 42572 ID: B2 Buffer: 0 0 0 0 11 1
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 42677 ID: 223 Buffer: C0 20 0 0 0 0 0 D 
// MB 0  OVERRUN: 0  LEN: 5 EXT: 0 TS: 43783 ID: 262 Buffer: 0 0 0 0 69
// MB 0  OVERRUN: 0  LEN: 3 EXT: 0 TS: 45472 ID: 20 Buffer: 0 0 7
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 45550 ID: B4 Buffer: 0 0 0 0 88 0 0 44
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 45673 ID: 24 Buffer: 2 1 1 F3 62 C 80 11
// MB 0  OVERRUN: 0  LEN: 8 EXT: 0 TS: 45794 ID: 25 Buffer: 0 E5 0 2 78 78 78 7C
// MB 0  OVERRUN: 0  LEN: 4 EXT: 0 TS: 45911 ID: 361 Buffer: 0 0 0 0