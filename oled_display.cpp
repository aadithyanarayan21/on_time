#include <include/display_subsystem/oled_display.h>

namespace display
{
    OLED_DISPLAY::OLED_DISPLAY()
    {
        this->sh1106_display_device_driver_handle = std::make_unique<SH1106_DISPLAY_DEVICE_DRIVER>(spi_path,
                                                                                                   gpio_chip_path,
                                                                                                   DATA_COMMAND_LINE_OFFSET,
                                                                                                   RESET_LINE_OFFSET
                                                                                                  );
    }


}
