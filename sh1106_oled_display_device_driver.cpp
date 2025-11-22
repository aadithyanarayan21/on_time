#include <include/display_subsystem/sh1106_oled_display_device_driver.h>

namespace display
{
    SH1106_DISPLAY_DEVICE_DRIVER::SH1106_DISPLAY_DEVICE_DRIVER(const std::filesystem::path& in_spi_path_ref,
                                                               const std::filesystem::path& in_gpiochip_path_ref,
                                                               const gpiod::line::offset&   in_data_command_line_offset,
                                                               const gpiod::line::offset&   in_reset_line_offset
                                                              )
        : dirty_page_tracker{},
          data_command_line_offset{in_data_command_line_offset},
          reset_line_offset{in_reset_line_offset}
    {
        this->gpio_chip_handle = std::make_unique<gpiod::chip>(in_gpiochip_path_ref);

        if (!this->gpio_chip_handle)
        {
            throw std::runtime_error("DISPLAY SUBSECTION: Failed to open GPIO chip");
        }

        try
        {
            this->data_command_line_handle = std::make_unique<gpiod::line_request>(this->gpio_chip_handle->prepare_request()
                                                                                           .set_consumer("oled_display_data_command_line")
                                                                                           .add_line_settings(in_data_command_line_offset,
                                                                                                              ::gpiod::line_settings().
                                                                                                                set_direction(::gpiod::line::direction::OUTPUT).
                                                                                                                set_bias(::gpiod::line::bias::PULL_UP).
                                                                                                                set_output_value(LOGIC_HIGH)  // Initial value HIGH
                                                                                                             )
                                                                                            .do_request()
                                                                                   );
        }
        catch(const std::system_error& e)
        {
            this->close_display();

            throw std::runtime_error(std::string("DISPLAY SUBSECTION: Failed to aquire data/command GPIO line: ") + e.what());
        }

        try
        {
            this->reset_line_handle = std::make_unique<gpiod::line_request>(this->gpio_chip_handle->prepare_request()
                                                                                    .set_consumer("oled_display_reset_line")
                                                                                    .add_line_settings(in_reset_line_offset,
                                                                                                       ::gpiod::line_settings().
                                                                                                       set_direction(::gpiod::line::direction::OUTPUT).
                                                                                                       set_bias(::gpiod::line::bias::PULL_UP).
                                                                                                       set_output_value(LOGIC_HIGH)  // Initial value HIGH
                                                                                                      )
                                                                                    .do_request()
                                                                            );
        }
        catch(const std::system_error& e)
        {
            this->close_display();

            throw std::runtime_error(std::string("DISPLAY SUBSECTION: Failed to aquire reset GPIO line: ") + e.what());
        }

        this->spi_channel_handle = std::make_unique<IO_CHANNEL::spi_channel>(in_spi_path_ref);

        // //set up the SPIO line...
        // if(this->spi_open(in_spi_path_ref) != 0)
        // {
        //     this->close_display();

        //     throw std::runtime_error("DISPLAY SUBSECTION: Failed to set up SPI channel");
        // }

        //Now that the display lines have been set up ... initialize the display.
        this->reset_display();

        this->display_buffer_handle = std::make_unique<BYTE[]>(SH1106_DISPLAY_PARAMETERS::TOTAL_DISPLAY_BUFFER_SIZE);
        this->clear_display_buffer();

    }

    SH1106_DISPLAY_DEVICE_DRIVER::~SH1106_DISPLAY_DEVICE_DRIVER()
    {
        this->close_display();
    }

    void SH1106_DISPLAY_DEVICE_DRIVER::close_display()
    {
        //if the display has valid data command line access then release it.
        if(this->data_command_line_handle)
        {
            this->data_command_line_handle.reset();
        }

        //if the display has valid reset line access then release it.
        if(this->reset_line_handle)
        {
            this->reset_line_handle.reset();
        }

        //finally close the display chip handle
        if (this->gpio_chip_handle)
        {
            this->gpio_chip_handle.reset();
        }
    }

    void SH1106_DISPLAY_DEVICE_DRIVER::clear_display_buffer()
    {
        std::fill(this->display_buffer_handle.get(),
                  this->display_buffer_handle.get() + (SH1106_DISPLAY_PARAMETERS::OLED_DISPLAY_MAX_ROWS * (SH1106_DISPLAY_PARAMETERS::OLED_DISPLAY_MAX_COLUMNS/SH1106_DISPLAY_PARAMETERS::ROWS_PER_PAGE)),
                  0x00
                 );
    }

     //Reset the display module. Blank the display and turn it off.
    void SH1106_DISPLAY_DEVICE_DRIVER::reset_display()
    {
        this->reset_line_handle->set_value(this->reset_line_offset,LOGIC_LOW);
        usleep(MILLISECONDS_10); // 10 ms
        this->reset_line_handle->set_value(this->reset_line_offset,LOGIC_HIGH);
        usleep(MILLISECONDS_10); // 10 ms

        //after resetting the display .. initialize it.
        this->init_display();
    }

    void SH1106_DISPLAY_DEVICE_DRIVER::mark_page_dirty(BYTE in_page)
    {
        // mark this page as dirty
        this->dirty_page_tracker |= (1 << in_page);
    }

    void SH1106_DISPLAY_DEVICE_DRIVER::mark_page_clean(BYTE in_page)
    {
        // clear this page as dirty
        this->dirty_page_tracker |= (1 << in_page);
    }

    void SH1106_DISPLAY_DEVICE_DRIVER::mark_all_pages_clean()
    {
        this->dirty_page_tracker = 0;
    }

    void SH1106_DISPLAY_DEVICE_DRIVER::set_page_address(BYTE in_page)
    {
       this->send_command(0xB0 | in_page);
    }

    void SH1106_DISPLAY_DEVICE_DRIVER::set_column_address(BYTE in_column)
    {
        this->send_command(0x00 | (in_column & 0x0F));
        this->send_command(0x10 | ((in_column >> 4) & 0x0F));
    }

    void SH1106_DISPLAY_DEVICE_DRIVER::send_command(BYTE in_command)
    {
        this->data_command_line_handle->set_value(this->data_command_line_offset, LOGIC_LOW); // D/C# = 0 for command
        this->spi_channel_handle->spi_write(&in_command, 1);
    }

    void SH1106_DISPLAY_DEVICE_DRIVER::send_data(BYTE in_data)
    {
        this->data_command_line_handle->set_value(this->data_command_line_offset, LOGIC_HIGH); // D/C# = 1 for data
        this->spi_channel_handle->spi_write(&in_data, 1);
    }

    void SH1106_DISPLAY_DEVICE_DRIVER::send_data_array(const BYTE* in_data, const uint16_t in_size)
    {
        if(in_size > SH1106_DISPLAY_PARAMETERS::TOTAL_DISPLAY_BUFFER_SIZE)
        {
            throw std::runtime_error("DISPLAY SUBSECTION: Message size exceeds max display capabilities");
        }

        this->data_command_line_handle->set_value(this->data_command_line_offset, LOGIC_HIGH);

        this->spi_channel_handle->spi_write(in_data, in_size);
    }

    bool SH1106_DISPLAY_DEVICE_DRIVER::init_display()
    {
        // Sequence: series of SH1106 init commands
        this->send_command(0xAE); // display off
        this->send_command(0xD5); this->send_command(0x80); // set display clock divide ratio/oscillator
        this->send_command(0xA8); this->send_command(0x3F); // multiplex 1/64
        this->send_command(0xD3); this->send_command(0x00); // display offset
        this->send_command(0x40); // set start line = 0
        this->send_command(0xAD); this->send_command(0x8B); // DC-DC on (Charge pump or external - SH1106 variant)
        this->send_command(0xA1); // segment remap
        this->send_command(0xC8); // COM scan dec
        this->send_command(0xDA); this->send_command(0x12); // COM pins
        this->send_command(0x81); this->send_command(0x80); // contrast
        this->send_command(0xD9); this->send_command(0x22); // precharge
        this->send_command(0xDB); this->send_command(0x40); // vcom detect
        this->send_command(0xA4); // display all on resume
        this->send_command(0xA6); // normal display
        this->send_command(0xAF); // display on

        this->clear_display_buffer();
        this->dirty_page_tracker = 0xFF;    // all 8 pages dirty
        this->flush_display_buffer();       // clear display...
        return true;
    }

    void SH1106_DISPLAY_DEVICE_DRIVER::flush_page(BYTE in_page)
    {
        if (in_page >= SH1106_DISPLAY_PARAMETERS::MAX_PAGES)
        {
            return; // invalid page
        }

        // Set page address
        this->set_page_address(in_page); // 0xB0 to 0xB7

        // Set column address to 2 (visible start)
        this->set_column_address(SH1106_DISPLAY_PARAMETERS::COLUMN_ORIGIN_OFFSET);

        // Calculate buffer offset
        int buffer_offset = in_page * SH1106_DISPLAY_PARAMETERS::OLED_DISPLAY_MAX_COLUMNS;

        // Send 128 bytes of data from buffer
        this->send_data_array(this->display_buffer_handle.get() + buffer_offset,
                              SH1106_DISPLAY_PARAMETERS::OLED_DISPLAY_MAX_COLUMNS
                             );

        this->mark_page_clean(in_page);
    }

    // Simple page-oriented flush for SH1106 (128x64 => 8 pages of 128 bytes)
    void SH1106_DISPLAY_DEVICE_DRIVER::flush_display_buffer()
    {
        int page{0};

        while (page < SH1106_DISPLAY_PARAMETERS::MAX_PAGES)
        {
            //if page is dirty then flush it...
            if (this->dirty_page_tracker & (1 << page))
            {
               this->flush_page(page);
            }

            ++page;
        }
    }

    // Draw one pixel (basic; origin top-left). SH1106 uses vertical pages;
    // this is a simple implementation.
    void SH1106_DISPLAY_DEVICE_DRIVER::display_pixel(BYTE in_row,
                                                     BYTE in_column,
                                                     PIXEL_STATE in_pixel_state
                                                    )
    {
        if ((in_row >= SH1106_DISPLAY_PARAMETERS::OLED_DISPLAY_MAX_ROWS)
            ||
            (in_column >= SH1106_DISPLAY_PARAMETERS::OLED_DISPLAY_MAX_ROWS)
           )
        {
            return; // do nothing
        }

        int page            = in_row / SH1106_DISPLAY_PARAMETERS::ROWS_PER_PAGE;
        int bit_position    = in_row % SH1106_DISPLAY_PARAMETERS::ROWS_PER_PAGE;
        int index           = (page*SH1106_DISPLAY_PARAMETERS::OLED_DISPLAY_MAX_COLUMNS)+in_column;

        if (in_pixel_state) //pixel on ...
        {
            *((this->display_buffer_handle.get())+index) |= (1 << bit_position);
        }
        else //pixel off
        {
            *((this->display_buffer_handle.get())+index) &= ~(1 << bit_position);
        }

        //mark this page dirty
        this->mark_page_dirty(page);
    }

    void SH1106_DISPLAY_DEVICE_DRIVER::display_page(BYTE in_page,
                                                    BYTE* in_buffer,
                                                    BYTE in_size)
    {


        if (in_page >= SH1106_DISPLAY_PARAMETERS::MAX_PAGES)
        {
            return; // invalid page
        }

        // Calculate buffer offset for this page
        // Clamp size to maximum columns (128)
        int buffer_offset = in_page * SH1106_DISPLAY_PARAMETERS::OLED_DISPLAY_MAX_COLUMNS;

        // Copy incoming data into display buffer
        std::copy(in_buffer,
                  in_buffer+ std::min(in_size,SH1106_DISPLAY_PARAMETERS::OLED_DISPLAY_MAX_COLUMNS),
                  this->display_buffer_handle.get() + buffer_offset
                );

        // Mark this page dirty
        this->dirty_page_tracker |= (1 << in_page);
    }
}






