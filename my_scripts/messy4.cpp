#include <Arduino.h>
#include <FlexCAN_T4.h>

FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> can1;
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> can2;

CAN_message_t tx_msg1, tx_msg2, rx_msg;

// My_CAN_error_t err1, err2;
CAN_error_t err1, err2;

elapsedMillis timer;
uint32_t last_send_time = 0;
// uint32_t send_interval = 10000; // 10 seconds
uint32_t send_interval = 5000; // 5 seconds

uint32_t id1 = 0x100;
uint32_t id2 = 0x200;
bool can1_turn = true;

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

void sendCANMessage(FlexCAN_T4_Base& can, CAN_message_t& msg, uint32_t& id, const char* label) {
  msg.id = id++;
  msg.len = 8;
  for (int i = 0; i < 8; i++) msg.buf[i] = i + id; // Vary data slightly
  can.write(msg);
  Serial.print("Sent from "); Serial.print(label); Serial.print(" → ID: 0x");
  Serial.println(msg.id, HEX);
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);
  Serial.println("Starting combined CAN test with error reporting");

  // can1.setTX(11);  // Or whatever pin you're using
  // can1.setRX(13);
  // can1.setTX(CAN1_TX11); // second try
  // can1.setRX(CAN1_RX13); // second try
  // can1.setTX(FLEXCAN1_TX11); // third try
  // can1.setRX(FLEXCAN1_RX13); // third try
  // can1.setTX(TX11); // fourth try
  // can1.setRX(RX13); // fourth try
  // can1.setTX(ALT); // TX11
  // can1.setRX(ALT); // RX13
  can1.setTX(DEF); // TX22
  can1.setRX(DEF); // RX23

  can1.begin();
  can1.setBaudRate(500000);
  // can1.enableLoopBack();

  can2.setTX(DEF);  // TX on pin 1
  can2.setRX(DEF);  // RX on pin 0

  can2.begin();
  can2.setBaudRate(500000);
  // can2.enableLoopBack();
}

void loop() {
  can1.events();
  can2.events();

  // === Periodic Message Sending ===
  if (millis() - last_send_time >= send_interval) {
    last_send_time = millis();

    if (can1_turn) {
      sendCANMessage(can1, tx_msg1, id1, "CAN1");
      // can1.error(err1, true);
      can1.error(err1, false);
      printCANError("CAN1", err1);
    } else {
      sendCANMessage(can2, tx_msg2, id2, "CAN2");
      // can2.error(err2, true);
      can2.error(err2, false);
      printCANError("CAN2", err2);
    }

    can1_turn = !can1_turn;
  }

  // === CAN Reading ===
  while (can1.read(rx_msg)) {
    Serial.print("Received on CAN1 → ID: 0x");
    Serial.print(rx_msg.id, HEX);
    Serial.print(" Data: ");
    for (int i = 0; i < rx_msg.len; i++) {
      Serial.print(rx_msg.buf[i], HEX); Serial.print(" ");
    }
    Serial.println();
  }

  while (can2.read(rx_msg)) {
    Serial.print("Received on CAN2 → ID: 0x");
    Serial.print(rx_msg.id, HEX);
    Serial.print(" Data: ");
    for (int i = 0; i < rx_msg.len; i++) {
      Serial.print(rx_msg.buf[i], HEX); Serial.print(" ");
    }
    Serial.println();
  }
}