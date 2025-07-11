#include <FlexCAN_T4.h>

FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> can1;
CAN_message_t msg;

void setup() {
  Serial.begin(115200);
  delay(300);

  can1.begin();
  can1.setBaudRate(500000);  // Standard 500k baud for most cars
  can1.enableFIFO();
  can1.enableFIFOInterrupt();
  can1.onReceive(printFrame);
}

void printFrame(const CAN_message_t &msg) {
  // SLCAN style: t1238AABBCCDDEEFF
  if (msg.flags.extended) {
    Serial.print('T'); // Extended frame
    Serial.printf("%08X", msg.id);
  } else {
    Serial.print('t'); // Standard frame
    Serial.printf("%03X", msg.id);
  }

  Serial.print(msg.len); // Number of data bytes

  for (int i = 0; i < msg.len; i++) {
    Serial.printf("%02X", msg.buf[i]);
  }

  Serial.println();
}

void loop() {
  can1.events(); // Poll CAN FIFO
}
