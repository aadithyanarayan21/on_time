from periphery import GPIO

# Updated mapping: signal -> (chip_path, line_offset)
pin_map = {
    "SEL": ("/dev/gpiochip1", 260),
    "D/C": ("/dev/gpiochip1", 192),
    "INC": ("/dev/gpiochip1", 196),
    "BEF": ("/dev/gpiochip1", 261),   # updated line
    "NXT": ("/dev/gpiochip0", 13),
    "Common": None                     # connected to GND
}

gpio_objects = {}

# Initialize GPIOs as input
for signal, mapping in pin_map.items():
    if mapping:
        chip, line = mapping
        try:
            gpio_objects[signal] = GPIO(chip, line, "in")
        except Exception as e:
            print(f"Error initializing {signal}: {e}")

# Read all GPIO pins once and print values
print("GPIO Readout:")
for signal, gpio_obj in gpio_objects.items():
    try:
        val = gpio_obj.read()
        print(f"{signal}: {val}")  # True = High, False = Low
    except Exception as e:
        print(f"Error reading {signal}: {e}")

# Close GPIOs
for gpio_obj in gpio_objects.values():
    gpio_obj.close()