import re
import uart_channel
class DateTime:
    def init(self):
        self.hour = 0
        self.minute = 0
        self.second = 0
        self.day = 0
        self.month = 0
        self.year = 0

class NavicTimeProcessor:
    def init(self):
        self.token_vector = []
        self.parsed_date_time = DateTime()

        # UART equivalent of:
        # std::make_unique<uart_channel>("/dev/ttyS0", B115200, 8, Parity::None, 1)
        self.uart_channel_handle = uart_channel(
            device="/dev/ttyS0",
            baud_rate=115200,
            char_size=8,
            parity='N',
            stop_bits=1
        )

    # ----------------------------------------------------
    # Clear date/time
    # ----------------------------------------------------
    def clear_date_time(self):
        self.parsed_date_time = DateTime()

    # ----------------------------------------------------
    # Split helper
    # ----------------------------------------------------
    def split(self, text, delimiter):
        self.token_vector = text.split(delimiter)

    # ----------------------------------------------------
    # Parse NMEA sentence
    # ----------------------------------------------------
    def parse(self, sentence):
        if not sentence or not sentence.startswith("$"):
            raise ValueError("Invalid NMEA sentence")

        self.split(sentence, ',')

        head = self.token_vector[0]

        if "RMC" in head:
            self.parse_rmc()
        elif "ZDA" in head:
            self.parse_zda()
        else:
            raise ValueError("Unsupported NMEA sentence type")

        return self.parsed_date_time

    # ----------------------------------------------------
    # Parse RMC Sentence
    # $GNRMC,hhmmss.sss,A,...,ddmmyy,...
    # ----------------------------------------------------
    def parse_rmc(self):
        self.clear_date_time()

        if len(self.token_vector) < 10:
            raise ValueError("Incomplete RMC sentence")

        timeStr = self.token_vector[1]
        dateStr = self.token_vector[9]

        # Time: hhmmss.sss
        self.parsed_date_time.hour   = int(timeStr[0:2])
        self.parsed_date_time.minute = int(timeStr[2:4])
        self.parsed_date_time.second = int(timeStr[4:6])

        # Date: ddmmyy
        self.parsed_date_time.day    = int(dateStr[0:2])
        self.parsed_date_time.month  = int(dateStr[2:4])
        self.parsed_date_time.year   = 2000 + int(dateStr[4:6])

    # ----------------------------------------------------
    # Parse ZDA Sentence
    # $GNZDA,hhmmss.sss,dd,mm,yyyy
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
