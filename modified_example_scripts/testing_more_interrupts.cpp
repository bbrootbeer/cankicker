#include <FlexCAN_T4.h>
FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> myCan;

// void canSniff(const CAN_message_t &msg);
void canKick(const CAN_message_t &msg);

void setup(void)
{
    Serial.begin(115200);
    // while (!Serial && millis() < 2000);  // wait for Serial to connect
    delay(1000); 
    Serial.println("Serial is alive");

    myCan.begin();
    myCan.setBaudRate(50000);
    myCan.setMaxMB(16);
    Serial.println("CAN initialized");

    // myCan.onReceive(canKick);

    // for (int i = 0; i < 32; i++) {
    //     myCan.setMB((FLEXCAN_MAILBOX)i, RX, EXT); // EXT mode catches both STD + EXT
    //     myCan.setMBFilter((FLEXCAN_MAILBOX)i, 0, 0); // Open mask = catch all IDs
    //     myCan.onReceive((FLEXCAN_MAILBOX)i, canKick); // Assign all to canKick
    // }

    myCan.enableFIFO();  // use FIFO instead of mailbox for now
    myCan.enableFIFOInterrupt();
    myCan.onReceive(canKick);

    // myCan.enableMBInterrupts(); // this line is NECESSARY

    // myCan.mailboxStatus();

    Serial.println("setup complete");



}

void loop()
{
    // CAN_message_t msg; // Declare a CAN message variable
    // Nothing here — mailbox interrupts call your callback automatically
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

// void maybeMaybeMaybe(const CAN_message_t &msg)
// {
//     char frameType = msg.flags.extended ? 'T' : 't';
//     Serial.print(frameType);
//     Serial.print(msg.id, HEX);
//     Serial.print(msg.len, HEX) for (uint8_t i = 0; i < msg.len; i++)
//     {
//         Serial.print(msg.buf[i], HEX);
//         Serial.print(" ");
//     }
//     Serial.println();
// }