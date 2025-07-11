#include <FlexCAN_T4.h>

// this is a fucntion prototype
void printCANError(const char* label, const CAN_error_t& err);

FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> can1;
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> can2;
CAN_message_t msg;
CAN_error_t err1, err2;

void setup(void) {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);  // wait for serial connection
  can1.begin();
  can1.setBaudRate(500000);
  // can1.error(err1, false);

  can2.begin();
  can2.setBaudRate(500000);
  // can2.error(err2, false);
}

void loop() {

  // Read CAN1 messages
  while (can1.read(msg)) {
    Serial.print("CAN1 ");
    Serial.print("MB: "); Serial.print(msg.mb);
    Serial.print("  ID: 0x"); Serial.print(msg.id, HEX);
    Serial.print("  EXT: "); Serial.print(msg.flags.extended);
    Serial.print("  LEN: "); Serial.print(msg.len);
    Serial.print(" DATA: ");
    for (uint8_t i = 0; i < msg.len; i++) {
      Serial.print(msg.buf[i]); Serial.print(" ");
    }
    Serial.print("  TS: "); Serial.println(msg.timestamp);
  }

  // can1.error(err1, false);
  // printCANError("CAN1", err1);
  // if (err1.state) printCANError("CAN1", err1);
  can1.error(err1, false);
  if (strcmp(err1.state, "Idle") != 0) {
    printCANError("CAN1", err1);
  }

  // Read CAN2 messages
  while (can2.read(msg)) {
    Serial.print("CAN2 ");
    Serial.print("MB: "); Serial.print(msg.mb);
    Serial.print("  ID: 0x"); Serial.print(msg.id, HEX);
    Serial.print("  EXT: "); Serial.print(msg.flags.extended);
    Serial.print("  LEN: "); Serial.print(msg.len);
    Serial.print(" DATA: ");
    for (uint8_t i = 0; i < msg.len; i++) {
      Serial.print(msg.buf[i]); Serial.print(" ");
    }
    Serial.print("  TS: "); Serial.println(msg.timestamp);
  }

  can2.error(err2, false);
  // printCANError("CAN2", err2);
  // if (err2.state) printCANError("CAN2", err2);
  // if (err1.state != CAN_ERROR_IDLE) printCANError("CAN1", err1);
  can2.error(err1, false);
  if (strcmp(err1.state, "Idle") != 0) {
    printCANError("CAN1", err1);
  }

}

// void printCANError(const char* label, const My_CAN_error_t& err) {
void printCANError(const char* label, const CAN_error_t& err) {
  Serial.print(label); Serial.print(" Error State: "); Serial.println(err.state);
  if (err.BIT1_ERR) Serial.println("  BIT1 Error");
  if (err.BIT0_ERR) Serial.println("  BIT0 Error");
  if (err.ACK_ERR)  Serial.println("  ACK Error");
  if (err.CRC_ERR)  Serial.println("  CRC Error");
  if (err.FRM_ERR)  Serial.println("  Frame Error");
  if (err.STF_ERR)  Serial.println("  Stuff Error");
  if (err.TX_WRN)   Serial.println("  TX Warning");
  if (err.RX_WRN)   Serial.println("  RX Warning");

  Serial.print("  FLT_CONF: "); Serial.println(err.FLT_CONF);
  Serial.print("  RX_ERR_COUNTER: "); Serial.println(err.RX_ERR_COUNTER);
  Serial.print("  TX_ERR_COUNTER: "); Serial.println(err.TX_ERR_COUNTER);
  Serial.print("  ESR1: 0x"); Serial.println(err.ESR1, HEX);
  Serial.print("  ECR: 0x"); Serial.println(err.ECR, HEX);
}
