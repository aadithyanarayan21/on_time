#pragma once

// sh1106_spi.cpp
// Minimal SH1106 SPI driver for Radxa Cubie A5E
// Dependencies: libgpiod, spidev

#include <include/display_subsystem/spi_channel.h>

using PIXEL_STATE   = bool;


inline constexpr uint32_t MILLISECONDS{1000}; //10 millisecond delay
inline constexpr uint32_t MILLISECONDS_10{10*MILLISECONDS};

inline constexpr gpiod::line::value LOGIC_LOW{gpiod::line::value::INACTIVE};
inline constexpr gpiod::line::value LOGIC_HIGH{gpiod::line::value::ACTIVE};

namespace display
{
    namespace SH1106_DISPLAY_PARAMETERS
    {
        //Display releated ...
        inline constexpr BYTE       OLED_DISPLAY_MAX_COLUMNS{128};  //128 pixels
        inline constexpr BYTE       OLED_DISPLAY_MAX_ROWS{64};      //64 pixels
        inline constexpr BYTE       ROWS_PER_PAGE{8};
        inline constexpr BYTE       COLUMN_ORIGIN_OFFSET{2};        //map 128 x 64 from 132 x 64 pins of SH1106
        inline constexpr u_int16_t  TOTAL_DISPLAY_BUFFER_SIZE{OLED_DISPLAY_MAX_COLUMNS*OLED_DISPLAY_MAX_ROWS};
        inline constexpr BYTE       MAX_PAGES{OLED_DISPLAY_MAX_ROWS/ROWS_PER_PAGE};
    }

    //SH1106 oled display controller.
    class SH1106_DISPLAY_DEVICE_DRIVER
    {
        private:
            BYTE                                 dirty_page_tracker;
            std::unique_ptr<BYTE[]>              display_buffer_handle;
            ::gpiod::line::offset                data_command_line_offset;
            ::gpiod::line::offset                reset_line_offset;

            std::unique_ptr<gpiod::chip>         gpio_chip_handle;
            std::unique_ptr<gpiod::line_request> data_command_line_handle;
            std::unique_ptr<gpiod::line_request> reset_line_handle;

            IO_CHANNEL::SPI_CHANNEL_HANDLE       spi_channel_handle;

            void set_page_address(BYTE in_page);
            void set_column_address(BYTE in_column);

            void mark_page_dirty(BYTE in_page);
            void mark_page_clean(BYTE in_page);
            void mark_all_pages_clean();

            bool init_display();
            bool set_command_mode();
            bool set_data_mode();

            void send_command(BYTE in_command);
            void send_data(BYTE in_data);
            void send_data_array(const BYTE* in_data, const uint16_t in_size);

            void close_display(); //release all resourses...

            // int  spi_open(const std::filesystem::path& in_spi_path_ref);
            // void spi_close();
            // void spi_write(const BYTE* in_data,  uint16_t in_size);

        public:
            SH1106_DISPLAY_DEVICE_DRIVER(const std::filesystem::path& in_spi_path_ref,
                                         const std::filesystem::path& in_gpiochip_path_ref,
                                         const ::gpiod::line::offset& in_data_command_line_offset,
                                         const ::gpiod::line::offset& in_reset_line_offset
                                        );

            ~SH1106_DISPLAY_DEVICE_DRIVER();

            void reset_display();    //resets the display and initializes the module parameters
            void clear_display_buffer();
            void flush_display_buffer();
            void flush_page(BYTE in_page);

            void display_pixel(BYTE in_row, BYTE in_column, PIXEL_STATE in_pixel_state);

            //max 128 bytes will be updated in the page
            void display_page(BYTE in_page,
                              BYTE* in_buffer,
                              BYTE in_size
                             );
    };

    using SH1106_DISPLAY_DEVICE_DRIVER_HANDLE = std::unique_ptr<SH1106_DISPLAY_DEVICE_DRIVER>;
}
