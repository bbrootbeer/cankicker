#include <FlexCAN_T4.h>

FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> can1;
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> can2;
CAN_message_t msg;

void setup(void) {
  // added serial monitoring
  Serial.begin(115200);
  while (!Serial && millis() < 3000);
  Serial.println("Starting combined CAN test with error reporting");
  
  // added the DEFs
  can1.setTX(DEF); // TX22
  can1.setRX(DEF); // RX23

  can1.begin();
  can1.setBaudRate(500000);
  
  // added the DEFs
  can2.setTX(DEF);  // TX on pin 1
  can2.setRX(DEF);  // RX on pin 0
  
  can2.begin();
  can2.setBaudRate(500000);
}

void loop() {
  // added .events()
  can1.events();
  can2.events();  

  if ( can1.read(msg) ) {
    Serial.print("CAN1 "); 
    Serial.print("MB: "); Serial.print(msg.mb);
    Serial.print("  ID: 0x"); Serial.print(msg.id, HEX );
    Serial.print("  EXT: "); Serial.print(msg.flags.extended );
    Serial.print("  LEN: "); Serial.print(msg.len);
    Serial.print(" DATA: ");
    for ( uint8_t i = 0; i < 8; i++ ) {
      Serial.print(msg.buf[i]); Serial.print(" ");
    }
    Serial.print("  TS: "); Serial.println(msg.timestamp);
  }
  else if ( can2.read(msg) ) {
    Serial.print("CAN2 "); 
    Serial.print("MB: "); Serial.print(msg.mb);
    Serial.print("  ID: 0x"); Serial.print(msg.id, HEX );
    Serial.print("  EXT: "); Serial.print(msg.flags.extended );
    Serial.print("  LEN: "); Serial.print(msg.len);
    Serial.print(" DATA: ");
    for ( uint8_t i = 0; i < 8; i++ ) {
      Serial.print(msg.buf[i]); Serial.print(" ");
    }
    Serial.print("  TS: "); Serial.println(msg.timestamp);
  }
}
