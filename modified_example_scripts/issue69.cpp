#include <FlexCAN_T4.h>
FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> can1;

void canKick(const CAN_message_t &msg);

void setup(void)
{
    Serial.begin(115200);
    delay(400);
    // pinMode(6, OUTPUT); digitalWrite(6, LOW); /* optional tranceiver enable pin */
    can1.begin();
    // can1.setBaudRate(1000000);
    can1.setBaudRate(500000);
    can1.setMaxMB(16);
    can1.enableFIFO();
    can1.enableFIFOInterrupt();
    can1.onReceive(canKick); // this is what calls your canKick() function, when new data comes in
    can1.mailboxStatus();
}

void canKick(const CAN_message_t &msg)
{
    Serial.print("MB ");
    Serial.print(msg.mb);
    Serial.print(" OVERRUN: ");
    Serial.print(msg.flags.overrun);
    Serial.print(" LEN: ");
    Serial.print(msg.len);
    Serial.print(" EXT: ");
    Serial.print(msg.flags.extended);
    Serial.print(" TS: ");
    Serial.print(msg.timestamp);
    Serial.print(" ID: ");
    Serial.print(msg.id, HEX);
    Serial.print(" Buffer: ");
    for (uint8_t i = 0; i < msg.len; i++)
    {
        Serial.print(msg.buf[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}

void loop()
{
    can1.events();

    static uint32_t timeout = millis();
    if (millis() - timeout > 200)
    {
        CAN_message_t msg;
        msg.id = random(0x1, 0x7FE);
        for (uint8_t i = 0; i < 8; i++)
            msg.buf[i] = i + 1;
        can1.write(msg);
        timeout = millis();
        can1.mailboxStatus();
    }
}

