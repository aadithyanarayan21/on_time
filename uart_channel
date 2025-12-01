from periphery import Serial
import time
import uart_channel
import re

class DateTime:
    def __init__(self):
        self.hour = 0
        self.minute = 0
        self.second = 0
        self.day = 0
        self.month = 0
        self.year = 0

class NavicTimeProcessor:
    def __init__(self, uart_port="/dev/ttyS0", baud=115200):
        self.token_vector = []
        self.parsed_date_time = DateTime()
        self.last_hw_time_ms = None
        # UART equivalent of:
        # std::make_unique<uart_channel>("/dev/ttyS0", B115200, 8, Parity::None, 1)
        self.uart_channel_handle = uart_channel(
            device="/dev/ttyS0",
            baud_rate=115200,
            char_size=8,
            parity='N',
            stop_bits=1
        )

        # Open UART like Radxa example
        self.serial = Serial(
        port=uart_port,
        baudrate=baud,
        databits=8,
        parity='N',
        stopbits=1
    )
        print(f"[NavIC UART] Opened {uart_port} @ {baud}")

    # ----------------------------------------------------
    def clear_date_time(self):
        self.parsed_date_time = DateTime()

    def split(self, text, delimiter):
        self.token_vector = text.split(delimiter)

        # ----------------------------------------------------
        # UART read + update only if Δt > 50ms
        # ----------------------------------------------------
        def read_and_update(self):
            """
            Reads UART (non-blocking), parses NMEA,
            and returns DateTime only if > 50 ms since last update.
            """

            # Non-blocking read check
            if self.serial.input_waiting() == 0:
                return None

            raw = self.serial.read(self.serial.input_waiting())

            try:
                line = raw.decode(errors="ignore").strip()
            except:
                line = str(raw).strip()

            if not line.startswith("$"):
                return None

            try:
                dt = self.parse(line)
            except:
                return None

            # Time filter
            now_ms = time.time() * 1000

            if self.last_hw_time_ms is None:
                self.last_hw_time_ms = now_ms
                return dt

            if (now_ms - self.last_hw_time_ms) > 50:
                self.last_hw_time_ms = now_ms
                return dt

            return None

        # ----------------------------------------------------
        def close(self):
            self.serial.close()


    # ----------------------------------------------------
    def parse(self, sentence):
        if not sentence.startswith("$"):
            raise ValueError("Invalid NMEA sentence")

        self.split(sentence, ',')

        head = self.token_vector[0]

        if "RMC" in head:
            self.parse_rmc()
        elif "ZDA" in head:
            self.parse_zda()
        else:
            raise ValueError("Unsupported NMEA sentence")

        return self.parsed_date_time

    # ----------------------------------------------------
    def parse_rmc(self):
        self.clear_date_time()
        if len(self.token_vector) < 10:
            raise ValueError("Incomplete RMC sentence")

        timeStr = self.token_vector[1]
        dateStr = self.token_vector[9]

        self.parsed_date_time.hour   = int(timeStr[0:2])
        self.parsed_date_time.minute = int(timeStr[2:4])
        self.parsed_date_time.second = int(timeStr[4:6])

        self.parsed_date_time.day    = int(dateStr[0:2])
        self.parsed_date_time.month  = int(dateStr[2:4])
        self.parsed_date_time.year   = 2000 + int(dateStr[4:6])

    # ----------------------------------------------------
    def parse_zda(self):
        self.clear_date_time()
        if len(self.token_vector) < 5:
            raise ValueError("Incomplete ZDA sentence")

        timeStr = self.token_vector[1]

        self.parsed_date_time.hour   = int(timeStr[0:2])
        self.parsed_date_time.minute = int(timeStr[2:4])
        self.parsed_date_time.second = int(timeStr[4:6])

        self.parsed_date_time.day    = int(self.token_vector[2])
        self.parsed_date_time.month  = int(self.token_vector[3])
        self.parsed_date_time.year   = int(self.token_vector[4])
