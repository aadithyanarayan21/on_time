#pragma once

#include <iostream>
#include <string>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <cstring>
#include <memory>

namespace navic
{
    enum class Parity
    {
        None,
        Even,
        Odd
    };

    class uart_channel
    {
        private:
            int fd;
            void configure(int in_baud_rate,
                           int in_char_size,
                           Parity in_parity,
                           int in_stop_bits
                          );

        public:
            explicit uart_channel(const std::string& device,
                                  int in_baud_rate,
                                  int in_char_size,
                                  Parity in_parity,
                                  int in_stop_bits
                                 );
            ~uart_channel();

            void write_data(const std::string& data);

            void read_data(char *buffer, size_t maxLen = 256);

    };

    using UART_CHANNEL_HANDLE = std::unique_ptr<uart_channel>;
}
