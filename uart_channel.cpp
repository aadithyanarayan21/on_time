#include <include/navic_subsystem/uart_channel.h>

namespace navic
{
    uart_channel::uart_channel(const std::string& device,
                               int in_baud_rate,
                               int in_char_size,
                               Parity in_parity,
                               int in_stop_bits
                               )
    {
        fd = open(device.c_str(), O_RDWR | O_NOCTTY | O_SYNC);

        if (fd < 0)
        {
            throw std::runtime_error("Error opening UART: " + std::string(strerror(errno)));
        }

        configure(in_baud_rate, in_char_size, in_parity, in_stop_bits);
    }

    uart_channel::~uart_channel()
    {
        if (fd >= 0)
        {
            close(fd);
        }
    }

    void uart_channel::configure(int in_baud_rate, int in_char_size, Parity in_parity, int in_stop_bits)
    {
        struct termios tty;

        if (tcgetattr(fd, &tty) != 0)
        {
            throw std::runtime_error("Error getting UART attributes: " + std::string(strerror(errno)));
        }

        // Baud rate
        cfsetospeed(&tty, in_baud_rate);
        cfsetispeed(&tty, in_baud_rate);

        // Character size
        tty.c_cflag &= ~CSIZE;
        switch (in_char_size)
        {
            case 5: tty.c_cflag |= CS5; break;
            case 6: tty.c_cflag |= CS6; break;
            case 7: tty.c_cflag |= CS7; break;
            case 8: tty.c_cflag |= CS8; break;
            default: throw std::invalid_argument("Unsupported char size");
        }

        // Parity
        if (in_parity == Parity::None)
        {
            tty.c_cflag &= ~PARENB;
        }
        else if (in_parity == Parity::Even)
        {
            tty.c_cflag |= PARENB;
            tty.c_cflag &= ~PARODD;
        }
        else if (in_parity == Parity::Odd)
        {
            tty.c_cflag |= PARENB;
            tty.c_cflag |= PARODD;
        }

        // Stop bits
        if (in_stop_bits == 1)
        {
            tty.c_cflag &= ~CSTOPB;
        }
        else if (in_stop_bits == 2)
        {
            tty.c_cflag |= CSTOPB;
        }
        else
        {
            throw std::invalid_argument("Unsupported stop bits");
        }

        // Other flags
        tty.c_cflag |= (CLOCAL | CREAD);
        tty.c_cflag &= ~CRTSCTS;
        tty.c_iflag &= ~(IXON | IXOFF | IXANY);
        tty.c_lflag = 0;
        tty.c_oflag = 0;

        tty.c_cc[VMIN]  = 1;
        tty.c_cc[VTIME] = 5;

        if (tcsetattr(fd, TCSANOW, &tty) != 0)
        {
            throw std::runtime_error("Error setting UART attributes: " + std::string(strerror(errno)));
        }
    }

    void uart_channel::write_data(const std::string& data)
    {
        int written = write(fd, data.c_str(), data.size());

        if (written < 0)
        {
            throw std::runtime_error("Error writing to UART: " + std::string(strerror(errno)));
        }
    }

    void uart_channel::read_data(char *buffer, size_t maxLen)
    {
        int n = read(fd, buffer, static_cast<int>(maxLen));

        if (n < 0)
        {
            throw std::runtime_error("Error reading from UART: " + std::string(strerror(errno)));
        }
    }
}
