#include <FlexCAN_T4.h>

FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> Can0;

void canKick(const CAN_message_t &msg);


void setup(void) {
  Serial.begin(115200); delay(400);
//   pinMode(6, OUTPUT); digitalWrite(6, LOW); /* optional tranceiver enable pin */
  Can0.begin();
  Can0.setBaudRate(500000);
  Can0.setMaxMB(32); // when using fifo this value doesn't apply to receive because RX is handled via dedicated fifo queue, not mailboxes...
//   Can0.enableFIFO(); // required for using the fifo receive queue
//   Can0.enableFIFOInterrupt(); // required for using the fifo receive queue
//   Can0.onReceive(canSniff);
  Can0.onReceive(canKick);
  // Can0.mailboxStatus();
}

void loop() {
  // Can0.events();

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