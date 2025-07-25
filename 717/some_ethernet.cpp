#include <FlexCAN_T4.h>
#include <stdint.h>
#include <stddef.h>
// #include <crc16_ccitt_table.h> // Assuming this is your custom CRC header.
// If you want to use the table-driven CRC, make sure this header defines 'crc16_table'
// If you want to use the bit-by-bit CRC, you don't strictly need a separate header for the table.

// --- NativeEthernet Includes ---
#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>

// --- Network Configuration ---
// IMPORTANT: Replace with a unique MAC address if you have multiple Teensy Ethernet devices
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEE }; // Example MAC address
IPAddress teensyIp(192, 168, 1, 100); // Teensy's static IP address
IPAddress gateway(192, 168, 1, 1);   // Your router/gateway IP
IPAddress subnet(255, 255, 255, 0); // Your network's subnet mask

// IP address of the PC or server you want to send UDP packets to
IPAddress remoteIp(192, 168, 1, 10); // <--- CHANGE THIS to your PC's IP address!
unsigned int remotePort = 12345;     // Port on your PC to listen on

EthernetUDP Udp; // An EthernetUDP instance to let us send and receive packets over UDP

// --- CAN Configuration ---
FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> Can0;

// --- CRC Functions (keeping yours as is, assuming they work for you) ---
// If crc16_ccitt_table is defined in crc16_ccitt_table.h, you don't need to redeclare it here.
// Assuming crc16_table is defined elsewhere or in the .h file you commented out.
extern const uint16_t crc16_table[256]; // Make sure this is globally available if used by crc16_ccitt_table function

uint16_t crc16_ccitt_table(const uint8_t* data, size_t len, uint16_t init = 0xFFFF) {
    uint16_t crc = init;
    for (size_t i = 0; i < len; ++i) {
        uint8_t tbl_idx = (crc >> 8) ^ data[i]; // high byte XOR input
        crc = (crc << 8) ^ crc16_table[tbl_idx];
    }
    return crc;
}

// Bit-by-bit CRC function (if you prefer this or don't have the table)
uint16_t crc16_ccitt(const uint8_t* data, size_t len, uint16_t poly = 0x1021, uint16_t init = 0xFFFF) {
    uint16_t crc = init;
    for (size_t i = 0; i < len; ++i) {
        crc ^= (uint16_t)data[i] << 8;
        for (uint8_t j = 0; j < 8; ++j) {
            if (crc & 0x8000)
                crc = (crc << 1) ^ poly;
            else
                crc <<= 1;
        }
    }
    return crc;
}
// --- End CRC Functions ---

void sevenOneSeven(const CAN_message_t &msg); // Forward declaration of your CAN receive function

void setup(void) {
    Serial.begin(115200);
    delay(400); // Give serial time to start up

    // --- Ethernet Setup ---
    Serial.println("Starting Ethernet...");
    // If you uncommented crc16_ccitt_table.h, ensure it doesn't conflict with Ethernet init.
    // Ethernet.init(10); // Optional: if you need to specify a CS pin for some SPI Ethernet shields (not usually needed for native)
    Ethernet.begin(mac, teensyIp, gateway, subnet); // Use static IP configuration

    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
        Serial.println("Ethernet hardware was not found. Please check wiring. :(");
        while (true) { delay(1); } // Halt if no Ethernet
    }
    if (Ethernet.linkStatus() == LinkOFF) {
        Serial.println("Ethernet cable is not connected. Please connect cable.");
        // You might want to loop here until link is up, or just continue and try to send anyway.
        // For this example, we'll continue, but send will fail.
    }

    Serial.print("Teensy IP address: ");
    Serial.println(Ethernet.localIP());

    // Start UDP listening (even if just sending, it's good practice for debugging/replies)
    Udp.begin(remotePort); // Teensy opens the port it *sends from* (or listens on)

    // --- CAN Setup ---
    Can0.begin();
    Can0.setBaudRate(500000);
    Can0.setMaxMB(32);
    Can0.onReceive(sevenOneSeven);
    Can0.enableMBInterrupts(); // enables all mailboxes to be interrupt enabled

    Serial.println("Setup complete.");
}

void loop() {
    // Can0.events(); // This is handled by the interrupt for onReceive
}

void sevenOneSeven(const CAN_message_t &msg)
{
    // The buffer size should be large enough to hold your entire packet.
    // Your current packet is 17 bytes (0xAA, 0x69, ID (4), len (1), data (8), CRC (2))
    uint8_t buffer[17];

    buffer[0] = 0xAA;
    buffer[1] = 0x69;

    // CAN ID (assuming your custom format for extended flag)
    uint32_t can_id = msg.id;
    if (msg.flags.extended) {
        can_id |= 0x80000000; // EFF flag
    }
    buffer[2] = can_id & 0xFF;
    buffer[3] = (can_id >> 8) & 0xFF;
    buffer[4] = (can_id >> 16) & 0xFF;
    buffer[5] = (can_id >> 24) & 0xFF;

    buffer[6] = msg.len; // CAN data length

    // Copy CAN data bytes
    for (uint8_t i = 0; i < 8; i++) { // CAN_MAX_DLEN is 8
        if (i < msg.len) {
            buffer[7 + i] = msg.buf[i]; // Copy actual data bytes
        } else {
            buffer[7 + i] = 0x00; // Pad unused data bytes with zeros
        }
    }

    // Calculate CRC over the relevant part of the buffer (0x69 up to padded CAN data)
    // The data to CRC is from buffer[1] (0x69) to buffer[14] (last CAN data byte)
    // This length is (14 - 1) + 1 = 14 bytes.
    uint16_t crc = crc16_ccitt_table(buffer + 1, 14); // Or crc16_ccitt if using bit-by-bit

    buffer[15] = (crc >> 8) & 0xFF; // CRC high byte
    buffer[16] = crc & 0xFF;       // CRC low byte

    // --- Send UDP Packet ---
    // Begin a UDP packet destined for remoteIp and remotePort
    Udp.beginPacket(remoteIp, remotePort);
    // Write your custom buffer into the UDP packet
    Udp.write(buffer, sizeof(buffer)); // Send all 17 bytes
    // End and send the packet
    Udp.endPacket();

    // You can still use Serial for debugging what's being sent
    // Serial.print("Sent UDP packet (len ");
    // Serial.print(sizeof(buffer));
    // Serial.print("): ");
    // for (uint8_t i = 0; i < sizeof(buffer); i++) {
    //     if (buffer[i] < 0x10) Serial.print("0");
    //     Serial.print(buffer[i], HEX);
    //     Serial.print(" ");
    // }
    // Serial.println();
}