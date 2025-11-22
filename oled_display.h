#pragma once
#include <include/display_subsystem/sh1106_oled_display_device_driver.h>

namespace display
{
    inline constexpr PIXEL_STATE PIXEL_OFF{false};
    inline constexpr PIXEL_STATE PIXEL_ON{true};

    //inline static const char* SPI_DEV{"/dev/spidev1.0"};
    //inline static const char* GPIO_CHIP{"gpiochip0"};
    //inline constexpr BYTE PIN_DC{6};
    //inline constexpr BYTE PIN_RESET{8}; // PI8 -> header pin 29

    // GPIO config - change chip name or line offsets if different on your board
    inline const std::filesystem::path gpio_chip_path("/dev/gpiochip0"); // adjust if your CS maps to .1
    inline const ::gpiod::line::offset DATA_COMMAND_LINE_OFFSET{6}; // PI6 -> header pin 27 (line offset within gpiochip0)
    inline const ::gpiod::line::offset RESET_LINE_OFFSET{8};        // PI8 -> header pin 29

    //SPI RELEATED ...
    inline const std::filesystem::path spi_path("/dev/spidev1.0"); // adjust this ...

    class OLED_DISPLAY
    {
        private:
            SH1106_DISPLAY_DEVICE_DRIVER_HANDLE sh1106_display_device_driver_handle;

        public:
            OLED_DISPLAY();
    };

    using OLED_DISPLAY_HANDLE = std::unique_ptr<OLED_DISPLAY>;
}

