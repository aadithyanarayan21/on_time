#include <include/display_subsystem/oled_display.h>


int main()
{
    //set up the display subsystem.

    display::OLED_DISPLAY_HANDLE oled_display_handle;

    try
    {
        oled_display_handle = std::make_unique<display::OLED_DISPLAY>();
    }
    catch(...)
    {

    }

    // demo: draw a diagonal line
    int i = 0;

    while(i < OLED_ROW_SIZE && i < OLED_COLUMN_SIZE)
    {
        oled_display_handle->drawPixel(i, i, pixel_on);
        ++i;
    }

    disp.flush();

    // keep program alive a bit so you can see the result; in real driver exit, you'd daemonize or return
    sleep(5);
    return 0;
}
