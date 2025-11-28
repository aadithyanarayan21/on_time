import serial
import re
# MANUALLY SET YOUR UART DEVICE
UART_PORT = "/dev/ttyS0"    # tty0 = /dev/ttyS0
BAUDRATE = 115200           # 8-N-1
# OPEN UART WITH 8-N-1
def open_uart():
    print(f"Opening {UART_PORT} at {BAUDRATE} baud (8N1)...")
    ser = serial.Serial(
        port=UART_PORT,
        baudrate=BAUDRATE,
        bytesize=serial.EIGHTBITS,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        timeout=1
    )
    return ser


# NMEA PARSER FUNCTIONS


def parse_gga(fields):
    return {
        "type": "GGA",
        "time": fields[1],
        "latitude": fields[2] + fields[3],
        "longitude": fields[4] + fields[5],
        "fix": fields[6],
        "satellites": fields[7],
        "hdop": fields[8],
        "altitude": fields[9] + " " + fields[10]
    }


def parse_rmc(fields):
    return {
        "type": "RMC",
        "time": fields[1],
        "validity": fields[2],
        "latitude": fields[3] + fields[4],
        "longitude": fields[5] + fields[6],
        "speed_knots": fields[7],
        "course": fields[8],
        "date": fields[9]
    }


def parse_nmea(sentence):
    if not sentence.startswith("$"):
        return None

    # Remove checksum
    sentence = sentence.split('*')[0]
    parts = sentence.split(',')

    if len(parts) < 2:
        return None

    msg_type = parts[0][3:]   # Skip GN/GG/GP etc.

    if msg_type == "GGA":
        return parse_gga(parts)
    elif msg_type == "RMC":
        return parse_rmc(parts)
    else:
        return {
            "type": msg_type,
            "raw_fields": parts[1:]
        }