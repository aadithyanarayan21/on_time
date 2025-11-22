# navic_time_processor.py
# NAVIC (PX1125 GNSS) NMEA date/time parser — Linux & PyCharm friendly

from dataclasses import dataclass
import serial
import threading
from typing import Optional, List


@dataclass
class DateTime:
    year: Optional[int] = None
    month: Optional[int] = None
    day: Optional[int] = None
    hour: Optional[int] = None
    minute: Optional[int] = None
    second: Optional[int] = None

    def is_valid(self):
        return None not in (
            self.year, self.month, self.day,
            self.hour, self.minute, self.second
        )

    def __str__(self):
        if not self.is_valid():
            return "<Invalid DateTime>"
        return f"{self.year:04d}-{self.month:02d}-{self.day:02d} " \
               f"{self.hour:02d}:{self.minute:02d}:{self.second:02d}"


class UARTReader:
    """Simple pyserial wrapper."""
    def __init__(self, port: str, baud: int = 115200, timeout: float = 1.0):
        self.port = port
        self.baud = baud
        self.timeout = timeout
        self.ser = None

    def open(self):
        self.ser = serial.Serial(self.port, self.baud, timeout=self.timeout)

    def close(self):
        if self.ser:
            self.ser.close()

    def readline(self) -> Optional[str]:
        if not self.ser:
            return None

        raw = self.ser.readline()
        if not raw:
            return None

        return raw.decode(errors="ignore").strip()


class NavicTimeProcessor:
    """Parses NMEA strings into usable date/time objects."""

    def __init__(self, uart_port="/dev/ttyS0", baud=115200):
        self.uart = UARTReader(uart_port, baud)
        self.running = False
        self._thread = None

    @staticmethod
    def split(line: str) -> List[str]:
        return line.split(",")

    def parse_rmc(self, tokens: List[str]) -> Optional[DateTime]:
        # RMC: $GNRMC,hhmmss.sss,A,lat,N,lon,E,spd,track,ddmmyy,...
        if len(tokens) < 10:
            return None

        t = tokens[1]
        d = tokens[9]

        if len(t) < 6 or len(d) < 6:
            return None

        hour = int(t[0:2])
        minute = int(t[2:4])
        second = int(t[4:6])

        day = int(d[0:2])
        month = int(d[2:4])
        year = int(d[4:6]) + 2000

        return DateTime(year, month, day, hour, minute, second)

    def parse_gga(self, tokens: List[str]) -> Optional[DateTime]:
        # GGA contains only time.
        if len(tokens) < 2:
            return None

        t = tokens[1]
        if len(t) < 6:
            return None

        hour = int(t[0:2])
        minute = int(t[2:4])
        second = int(t[4:6])

        # date unknown
        return DateTime(None, None, None, hour, minute, second)

    def process_line(self, line: str) -> Optional[DateTime]:
        if not line.startswith("$"):
            return None

        payload = line[1:].split("*")[0]
        tokens = self.split(payload)

        header = tokens[0]

        if header.endswith("RMC"):
            return self.parse_rmc(tokens)

        if header.endswith("GGA"):
            return self.parse_gga(tokens)

        return None

    def start(self, callback):
        """Starts background reading of UART + callback(DateTime)."""
        self.uart.open()
        self.running = True

        def worker():
            while self.running:
                line = self.uart.readline()
                if line:
                    dt = self.process_line(line)
                    if dt and dt.is_valid():
                        callback(dt)

        self._thread = threading.Thread(target=worker, daemon=True)
        self._thread.start()

    def stop(self):
        self.running = False
        if self._thread:
            self._thread.join(timeout=1)
        self.uart.close()
