#Step 1 : Import the libraries
 from periphery import SPI,GPIO
 import time
 from datetime import datetime,timedelta

#------------------------------------------------------------------------------------------

#Step 2 : Initialise SPI interface and SSH1106 OLED
SPI_DEV = "/dev/spidev1.0"
SPI_MODE = 0
SPI_BAUD = 1000000
DC_PIN = 27
RST_PIN = 29
CMD_SET_PHASE_LEN = 0xB1
class SSH1106:
    def __init__(self, spi_dev=SPI_DEV, mode=SPI_MODE, baud=SPI_BAUD, dc_pin=DC_PIN, rst_pin=RST_PIN):
        # init SPI
        self.spi = SPI(spi_dev, mode, baud)
        # init gpio
        self.dc = GPIO(dc_pin, "out")
        self.rst = GPIO(rst_pin, "out")
        # reset sequence
        self.reset()
        # run init sequence
        self.init_display()

    def reset(self):
        # active low reset
        self.rst.write(True)
        time.sleep(0.01)
        self.rst.write(False)
        time.sleep(0.01)
        self.rst.write(True)
        time.sleep(0.01)

    def write_cmd(self, buf):
        # DC low for command
        self.dc.write(False)
        # spi.transfer expects list of ints or bytes-like
        self.spi.transfer(list(buf))

    def write_data(self, buf):
        # DC high for data
        self.dc.write(True)
        self.spi.transfer(list(buf))

    def init_display(self):
        # conservative init sequence for SH1106-like controllers
        cmds = [
            0xAE,             # DISPLAYOFF
            0xD5, 0x50,       # Set display clock divide/oscillator
            0xA8, 0x3F,       # Set multiplex ratio (1 to 64) -> 0x3F for 64
            0xD3, 0x00,       # Set display offset
            0x40,             # Set start line = 0
            0xAD, 0x8B,       # DC-DC on (charge pump) - module dependent; try 0x8B or 0x80 if 0x8B fails
            0xA1,             # segment remap
            0xC8,             # COM scan direction
            0xDA, 0x12,       # COM pins hardware config
            0x81, 0x7F,       # contrast
            0xD9, 0x22,       # pre-charge
            0xDB, 0x40,       # VCOM detect
            0xA4,             # display all off (resume)
            0xA6,             # normal display (not inverted)
            0xAF              # DISPLAYON
        ]
        # send as commands
        for b in cmds:
            self.write_cmd([b])
            # small delay between commands
            time.sleep(0.001)

    def close(self):
        try:
            self.spi.close()
        except:
            pass
        try:
            self.dc.close()
            self.rst.close()
        except:
            pass

        def send_framebuffer(self, fb_bytes):
            if len(fb_bytes) != 128 * 8:
                raise ValueError("fb_bytes must be exactly 1024 bytes")

            # Page addressing loop
            for page in range(8):  # pages 0..7
                # set page address (0xB0 + page)
                self.write_cmd([0xB0 | page])
                # set lower column address
                self.write_cmd([0x02])  # column low nibble (SH1106 often needs offset 2)
                # set higher column address
                self.write_cmd([0x10])

                page_offset = page * 128
                page_data = fb_bytes[page_offset:page_offset + 128]
                # send page data
                self.write_data(page_data)

#----------------------------------------------------------------------------------------------

#Step 3 : Start a loop to represent display time

#----------------------------------------------------------------------------------------------

#Step 4 : Define a template of time constant string in python (ROM DATA)
FONT_8x8 = {
    " ": [0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00],
    "=": [0x00,0x14,0x14,0x14,0x14,0x00,0x00,0x00],
    ":": [0x00,0x00,0x06,0x06,0x00,0x06,0x06,0x00],
    "0": [0x3C,0x66,0x6E,0x76,0x66,0x66,0x3C,0x00],
    "1": [0x18,0x38,0x18,0x18,0x18,0x18,0x3C,0x00],
    "2": [0x3C,0x66,0x06,0x0C,0x18,0x30,0x7E,0x00],
    "3": [0x3C,0x66,0x06,0x1C,0x06,0x66,0x3C,0x00],
    "4": [0x0C,0x1C,0x3C,0x6C,0x7E,0x0C,0x0C,0x00],
    "5": [0x7E,0x60,0x7C,0x06,0x06,0x66,0x3C,0x00],
    "6": [0x1C,0x30,0x60,0x7C,0x66,0x66,0x3C,0x00],
    "7": [0x7E,0x66,0x06,0x0C,0x18,0x18,0x18,0x00],
    "8": [0x3C,0x66,0x66,0x3C,0x66,0x66,0x3C,0x00],
    "9": [0x3C,0x66,0x66,0x3E,0x06,0x0C,0x38,0x00],
    "C": [0x3C,0x66,0x60,0x60,0x60,0x66,0x3C,0x00],
    "l": [0x18,0x18,0x18,0x18,0x18,0x18,0x0C,0x00],
    "k": [0x66,0x6C,0x78,0x70,0x78,0x6C,0x66,0x00],
    "I": [0x3C,0x18,0x18,0x18,0x18,0x18,0x3C,0x00],
    "S": [0x3C,0x66,0x60,0x3C,0x06,0x66,0x3C,0x00],
    "T": [0x7E,0x5A,0x18,0x18,0x18,0x18,0x3C,0x00],
}

def glyph_for_char(ch):
    return FONT_8x8.get(ch, FONT_8x8[" "])

#---------------------------------------------------------------------------------------

#Step 5 : Creating RAM framebuffer called anvil buffer ( size - 128x64 Bytes )
FB_WIDTH = 128
FB_PAGES = 8
FB_BYTES = FB_WIDTH * FB_PAGES

def clear_fb():
    return bytearray([0x00] * FB_BYTES)

#---------------------------------------------------------------------------------------

#Step 6 : Copy ROM buffer to Anvil buffer
def render_text_to_fb(fb, text, start_col=0, start_page=0):
    col = start_col
    page = start_page
    for ch in text:
        if col + 8 > FB_WIDTH:
            break  # no wrap
        glyph = glyph_for_char(ch)
        # glyph is list of 8 bytes; each byte represents 8 vertical pixels for that column
        # Page addressing: each page is 8 vertical pixels; the glyph fits in a single page (8px tall)
        page_offset = page * FB_WIDTH
        for i in range(8):
            fb[page_offset + col + i] = glyph[i]
        col += 8

#----------------------------------------------------------------------------------------

#Step 7 : Read from linux time information
def current_ist_time_tuple():
    utc = datetime.utcnow()
    ist = utc + timedelta(hours=5, minutes=30)
    return ist.timetuple()

#------------------------------------------------------------------------------------------

#Step 8 : Update anvil buffer with time information
def build_clock_string(ist_dt):
    hh = f"{ist_dt.tm_hour:02d}"
    mm = f"{ist_dt.tm_min:02d}"
    ss = f"{ist_dt.tm_sec:02d}"
    # Form exactly: Clk= XX:XX:XXIST  (no extra space between time and IST)
    return f"Clk={hh}:{mm}:{ss}IST"

#------------------------------------------------------------------------------------------------------

#Step 9 : Getting Glyph for each character of the anvil buffer and send that glyph to display via spi
def main():
    oled = SSH1106()
    try:
        # initial fb
        fb = clear_fb()
        # Pre-render static label if you want on top or bottom (we render full string each cycle)
        while True:
            # Read time in IST
            ist = current_ist_time_tuple()
            text = build_clock_string(ist)

            # Step 6 (again): copy ROM -> anvil buffer (i.e. clear and render)
            fb = clear_fb()
            # place the text centered vertically and horizontally — we choose top page 0 (you can change)
            # choose page 2 to roughly center text vertically: page 2 (y=16..23) etc — adjust as needed
            render_text_to_fb(fb, text, start_col=0, start_page=2)
            oled.send_framebuffer(fb)

#--------------------------------------------------------------------------------------------------------

#Step 10: wait for 500ms
time.sleep(0.5)

#------------------------------------------------------------------------------------------------------------
#Step 11: Go to Step 6
except KeyboardInterrupt:
        print("Exiting on user interrupt")
    finally:
        oled.close()

if __name__ == "__main__":
    print("Starting SSH1106 Clock (Clk= XX:XX:XXIST)")
    main()