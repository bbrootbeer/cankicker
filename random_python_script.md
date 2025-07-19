import serial
import socket

SERIAL_PORT = 'COM3'      # Change to your serial port, e.g., '/dev/ttyUSB0' on Linux
BAUD_RATE = 115200
UDP_IP = '127.0.0.1'      # Change to your target IP (PC IP)
UDP_PORT = 5005

def parse_can_frame(data):
    # Data format (excluding SOF 0xAA):
    # [0] = flags (bit0 = extended)
    # [1..4] = 4 bytes CAN ID (big endian)
    # [5] = length
    # [6..(6+len-1)] = payload
    # [..] = checksum (last byte)
    extended_flag = data[0]
    is_extended = bool(extended_flag & 0x01)
    can_id = (data[1] << 24) | (data[2] << 16) | (data[3] << 8) | data[4]
    length = data[5]
    payload = data[6:6 + length]
    checksum = data[6 + length]

    # Verify checksum (XOR of all bytes except SOF)
    calc_checksum = 0
    for b in data[:-1]:
        calc_checksum ^= b
    if calc_checksum != checksum:
        print("Checksum mismatch!")
        return None

    return {
        'extended': is_extended,
        'id': can_id,
        'length': length,
        'data': payload,
    }

def main():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)

    print(f"Listening on serial port {SERIAL_PORT} and sending UDP to {UDP_IP}:{UDP_PORT}")

    while True:
        byte = ser.read(1)
        if not byte:
            continue
        if byte[0] == 0xAA:  # SOF marker
            frame = ser.read(15)  # fixed frame length in your code
            if len(frame) != 15:
                print("Incomplete frame received")
                continue
            can_frame = parse_can_frame(frame)
            if can_frame is None:
                continue

            # Build a simple UDP payload (e.g., packed bytes)
            # Here we just send raw CAN frame bytes as example
            udp_payload = bytearray()
            udp_payload.append(0xAA)
            udp_payload.extend(frame)

            sock.sendto(udp_payload, (UDP_IP, UDP_PORT))
            print(f"Sent CAN ID {hex(can_frame['id'])} Length {can_frame['length']} Data {can_frame['data'].hex()}")

if __name__ == "__main__":
    main()
