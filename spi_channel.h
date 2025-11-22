#pragma once

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <string.h>
#include <gpiod.hpp>
#include <memory>

using BYTE          = uint8_t;
inline constexpr BYTE BITS_PER_WORD{8};

inline constexpr uint32_t MHz_1{1000 * 1000};
inline constexpr uint32_t MHz_10{10*MHz_1};
inline constexpr uint32_t SPEED{MHz_10};

namespace IO_CHANNEL
{
    inline constexpr BYTE SPI_MODE{SPI_MODE_0};

    class spi_channel
    {
        private:
            int  spi_fd; //SPI File descriptor

            int  spi_open(const std::filesystem::path& in_spi_path_ref);
            void spi_close();
        public:
            spi_channel(const std::filesystem::path& in_spi_path_ref);
            ~spi_channel();

            void spi_write(const BYTE* in_data,  uint16_t in_size);
    };

    using SPI_CHANNEL_HANDLE = std::unique_ptr<spi_channel>;
}

