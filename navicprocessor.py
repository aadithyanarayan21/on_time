import serial

class ParsedDateTime:
    def init(self):
        self.hour = 0
        self.minute = 0
        self.second = 0
        self.day = 0
        self.month = 0
        self.year = 0


class NavicTimeProcessor:
    def init(self):
        self.token_list = []
        self.parsed_date_time = ParsedDateTime()

        # Equivalent to uart_channel("/dev/ttyS0", 115200, 8, None, 1)
        self.serial_port = serial.Serial(
            port="/dev/ttyS0",
            baudrate=115200,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            timeout=1
        )

    def clear_date_time(self):
        self.parsed_date_time = ParsedDateTime()

    def split_string(self, input_str, delimiter=','):
        """ Equivalent to tokenizing a CSV-like string """
        self.token_list = input_str.split(delimiter)

    def parse_time_message(self):
        """
        C++ logic:
            token[1] = hhmmss.sss
            hour   = first 2 chars
            minute = next 2 chars
            second = next 2 chars
            date   = token[2]
            month  = token[3]
            year   = token[4]
        """
        if len(self.token_list) < 5:
            print("Invalid input, not enough tokens")
            return

        time_str = self.token_list[1]  # hhmmss.sss

        self.parsed_date_time.hour = int(time_str[0:2])
        self.parsed_date_time.minute = int(time_str[2:4])
        self.parsed_date_time.second = int(time_str[4:6])

        self.parsed_date_time.day = int(self.token_list[2])
        self.parsed_date_time.month = int(self.token_list[3])
        self.parsed_date_time.year = int(self.token_list[4])

    def print_datetime(self):
        dt = self.parsed_date_time
        print(f"{dt.day:02d}-{dt.month:02d}-{dt.year}  {dt.hour:02d}:{dt.minute:02d}:{dt.second:02d}")
