from luma.core.interface.serial import spi
from luma.oled.device import sh1106
from luma.core.render import canvas
from PIL import ImageFont

# ------------------------------------------
# Configuration according to your wiring
# ------------------------------------------

# D/C and RESET GPIO numbers (modify only if needed)
DC_PIN = 16       # DISP_D/C (PL12 from schematic example)
RESET_PIN = 8    # DISP_RESET (PI16 from schematic)

# Create SPI interface using /dev/spidev1.0
serial = spi(
    port=1,
    device=0,
    gpio_DC=DC_PIN,
    gpio_RST=RESET_PIN,
    bus_speed_hz=1000000  # 1 MHz, same as your SPI test code
)

# Create OLED SH1106 device
device = sh1106(serial, width=128, height=64, rotate=0)

# ------------------------------------------
# Load Font (Sans-serif 5x7 or 8x8)
# ------------------------------------------

# Try to load built-in PIL bitmap font
try:
    font = ImageFont.load_default()  # closest to sans-serif 5x7 style
except:
    font = None  # fallback if missing

# ------------------------------------------
# Display Hello World
# ------------------------------------------

with canvas(device) as draw:
    draw.text((10, 28), "HELLO WORLD", font=font, fill=255)

print("OLED message displayed successfully!")
