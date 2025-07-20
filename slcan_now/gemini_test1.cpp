#include <FlexCAN_T4.h>

// Instantiate the CAN object. CAN1 is typically used on Teensy 4.x.
FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> Can1;

void slcand1(const CAN_message_t &msg);


// Buffer to store incoming serial commands from the PC
char serialRxBuffer[64];
uint8_t serialRxBufferIndex = 0;

// Function prototypes
void slcand1(const CAN_message_t &msg); // Callback for received CAN messages (for SavvyCAN)
void canss(const CAN_message_t &msg); // Callback for debug printing (for Arduino Serial Monitor)
void processSlcanCommand(); // Function to process incoming serial commands

void setup(void) {
  // Initialize USB Serial communication. SavvyCAN will connect to this.
  Serial.begin(115200); // Standard serial baud rate for slcan
  delay(500); // Give some time for Serial to initialize

  // Initialize CAN bus. HARDCODE YOUR CAN BUS BAUD RATE HERE.
  // This sketch does NOT implement the 'S' command to set it dynamically.
  Can1.begin();
  Can1.setBaudRate(500000); // <--- IMPORTANT: SET THIS TO YOUR CAR'S CAN BAUD RATE (e.g., 500000, 250000, 125000)
  Can1.setMaxMB(16); // Use a reasonable number of mailboxes

  Can1.onReceive(slcand1); // Register callback for received CAN messages (PRIMARY FOR SAVVYCAN)
  // Can1.onReceive(canss); // <--- COMMENTED OUT: UNCOMMENT THIS LINE ONLY FOR DEBUGGING IN ARDUINO SERIAL MONITOR
  Can1.enableMBInterrupts(); // enables all mailboxes to be interrupt enabled
}

void loop() {
  // Check for incoming serial data from the PC
  while (Serial.available()) {
    char incomingChar = Serial.read();
    if (incomingChar == '\r') { // Command ends with carriage return
      serialRxBuffer[serialRxBufferIndex] = '\0'; // Null-terminate the string
      processSlcanCommand(); // Process the received command
      serialRxBufferIndex = 0; // Reset buffer for next command
    } else if (serialRxBufferIndex < sizeof(serialRxBuffer) - 1) {
      // Add character to buffer, prevent overflow
      serialRxBuffer[serialRxBufferIndex++] = incomingChar;
    }
  }
  // Can1.events(); // Not strictly necessary with onReceive and interrupts enabled
}

/**
 * @brief Callback function executed when a CAN message is received.
 * Formats the received message into an SLCAN string and sends it over Serial.
 *
 * SLCAN receive format for standard ID: t<ID_HEX_3_DIGITS>#<DATA_HEX_BYTES>\r
 * SLCAN receive format for extended ID: T<ID_HEX_8_DIGITS>#<DATA_HEX_BYTES>\r
 */
void slcand1(const CAN_message_t &msg) {
  // Determine frame type character based on whether it's extended
  char frameType = msg.flags.extended ? 'T' : 't';
  Serial.print(frameType);

  // Print ID in correct width (3 digits for standard, 8 for extended)
  if (msg.flags.extended) {
    Serial.printf("%08lX", msg.id); // %lX for long hexadecimal
  } else {
    Serial.printf("%03lX", msg.id); // %lX for long hexadecimal
  }

  // Add the data length delimiter '#'
  Serial.print('#');

  // Print data bytes (each 2 hex digits)
  for (uint8_t i = 0; i < msg.len; i++) {
    Serial.printf("%02X", msg.buf[i]);
  }

  // End of SLCAN frame (important)
  Serial.print('\r');
}

/**
 * @brief Secondary callback for debugging, prints verbose CAN message info to Serial.
 * This function should typically be disabled when using SavvyCAN to avoid serial corruption.
 */
void canss(const CAN_message_t &msg) {
  Serial.print("MB "); Serial.print(msg.mb);
  Serial.print("  OVERRUN: "); Serial.print(msg.flags.overrun);
  Serial.print("  LEN: "); Serial.print(msg.len);
  Serial.print(" EXT: "); Serial.print(msg.flags.extended);
  Serial.print(" TS: "); Serial.print(msg.timestamp);
  Serial.print(" ID: "); Serial.print(msg.id, HEX);
  Serial.print(" Buffer: ");
  for ( uint8_t i = 0; i < msg.len; i++ ) {
    Serial.print(msg.buf[i], HEX); Serial.print(" ");
  } Serial.println();
}


/**
 * @brief Processes an incoming SLCAN command from the serial buffer.
 * Supports basic SLCAN commands for transmitting CAN frames:
 * t<ID>#<DATA>: Transmit standard CAN frame
 * T<ID>#<DATA>: Transmit extended CAN frame
 *
 * It also includes basic 'O' (Open) and 'C' (Close) command acknowledgements,
 * and 'S' (Set Baud Rate) acknowledgements, even though the baud rate is hardcoded.
 * This helps SavvyCAN interact more smoothly without errors.
 */
void processSlcanCommand() {
  char commandChar = serialRxBuffer[0];
  CAN_message_t txMsg;
  txMsg.len = 0; // Default to 0 length

  switch (commandChar) {
    case 'S': // Set Baud Rate (respond with ACK, but baud rate is hardcoded)
      Serial.print('\r'); // Acknowledge command
      break;

    case 'O': // Open CAN Channel (respond with ACK, but channel is always open)
      Serial.print('\r'); // Acknowledge command
      break;

    case 'C': // Close CAN Channel (respond with ACK, but channel is always open)
      Serial.print('\r'); // Acknowledge command
      break;

    case 't': // Transmit Standard CAN Frame (11-bit ID)
    case 'T': // Transmit Extended CAN Frame (29-bit ID)
      txMsg.flags.extended = (commandChar == 'T'); // Set extended flag

      // Parse ID
      char* hashPtr = strchr(serialRxBuffer, '#');
      if (hashPtr == NULL) {
        Serial.print('\a'); // Error: No '#' found
        Serial.print('\r'); // End response
        return;
      }
      *hashPtr = '\0'; // Temporarily null-terminate ID string
      txMsg.id = strtol(&serialRxBuffer[1], NULL, 16); // Parse ID as hex

      // Parse Data
      char* dataPtr = hashPtr + 1;
      int dataLen = strlen(dataPtr);
      if (dataLen % 2 != 0) {
        Serial.print('\a'); // Error: Data length must be even (hex pairs)
        Serial.print('\r'); // End response
        return;
      }
      txMsg.len = dataLen / 2;
      if (txMsg.len > 8) { // Max 8 bytes for CAN 2.0
        Serial.print('\a'); // Error: Data too long
        Serial.print('\r'); // End response
        return;
      }

      for (uint8_t i = 0; i < txMsg.len; i++) {
        char byteStr[3];
        byteStr[0] = dataPtr[i * 2];
        byteStr[1] = dataPtr[i * 2 + 1];
        byteStr[2] = '\0';
        txMsg.buf[i] = strtol(byteStr, NULL, 16); // Parse each byte as hex
      }

      // Attempt to transmit
      if (Can1.write(txMsg)) {
        Serial.print('z'); // Acknowledge successful transmit
      } else {
        Serial.print('\a'); // Bell character for transmit error
      }
      Serial.print('\r'); // End of response
      break;

    case 'r': // Read status flags (respond with ACK for simplicity)
      Serial.print('\r');
      break;

    default: // Unknown command
      Serial.print('\a'); // Bell character for unknown command
      Serial.print('\r');
      break;
  }
}