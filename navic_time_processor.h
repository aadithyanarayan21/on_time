#ifndef NAVIC_TIME_PROCESSOR_H
#define NAVIC_TIME_PROCESSOR_H

#include <include/navic_subsystem/uart_channel.h>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <iomanip>



namespace navic
{
    struct DateTime
    {
        int hour;
        int minute;
        int second;
        int day;
        int month;
        int year;
    };

    class navic_time_processor
    {
        private:
            UART_CHANNEL_HANDLE         uart_channel_handle;
            DateTime                    parsed_date_time;
            std::vector<std::string>    token_vector;


            void                        clear_date_time();

            void                        split(const std::string& str, char delimiter);
            void                        parseRmc();
            void                        parseZda();

        public:
        navic_time_processor();

        DateTime& parse(const std::string& sentence);
    };
}

#endif // NAVIC_TIME_PROCESSOR_H

#if 0
// Example usage
int main()
{
    try
    {
        NmeaParser parser;

        // Example NAVIC NMEA sentences
        std::string rmc = "$GNRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,151125,,,A*68";
        std::string zda = "$GNZDA,123519.00,15,11,2025,00,00*57";

        auto dt1 = parser.parse(rmc);
        std::cout << "RMC Time: " << dt1.hour << ":" << dt1.minute << ":" << dt1.second
                  << " Date: " << dt1.day << "-" << dt1.month << "-" << dt1.year << std::endl;

        auto dt2 = parser.parse(zda);
        std::cout << "ZDA Time: " << dt2.hour << ":" << dt2.minute << ":" << dt2.second
                  << " Date: " << dt2.day << "-" << dt2.month << "-" << dt2.year << std::endl;
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
    }

    return 0;
}
#endif

#if 0
#include <iostream>
#include <string>
#include <stdexcept>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <vector>

// -------------------- Parity Enum --------------------
enum class Parity
{
    None,
    Even,
    Odd
};

// -------------------- UART Class --------------------
class Uart
{
public:
    Uart(const std::string& device,
         int baudRate,
         int charSize,
         Parity parity,
         int stopBits)
    {
        fd = open(device.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
        if (fd < 0)
        {
            throw std::runtime_error("Error opening UART: " + std::string(strerror(errno)));
        }

        configure(baudRate, charSize, parity, stopBits);
    }

    ~Uart()
    {
        if (fd >= 0)
        {
            close(fd);
        }
    }

    std::string readLine()
    {
        std::string line;
        char ch;
        while (true)
        {
            int n = read(fd, &ch, 1);
            if (n > 0)
            {
                if (ch == '\n')
                {
                    break;
                }
                line += ch;
            }
        }
        return line;
    }

    void writeData(const std::string& data)
    {
        int written = write(fd, data.c_str(), data.size());
        if (written < 0)
        {
            throw std::runtime_error("Error writing to UART: " + std::string(strerror(errno)));
        }
    }

private:
    int fd;

    void configure(int baudRate, int charSize, Parity parity, int stopBits)
    {
        struct termios tty;
        if (tcgetattr(fd, &tty) != 0)
        {
            throw std::runtime_error("Error getting UART attributes: " + std::string(strerror(errno)));
        }

        cfsetospeed(&tty, baudRate);
        cfsetispeed(&tty, baudRate);

        tty.c_cflag &= ~CSIZE;
        switch (charSize)
        {
            case 5: tty.c_cflag |= CS5; break;
            case 6: tty.c_cflag |= CS6; break;
            case 7: tty.c_cflag |= CS7; break;
            case 8: tty.c_cflag |= CS8; break;
            default: throw std::invalid_argument("Unsupported char size");
        }

        if (parity == Parity::None)
        {
            tty.c_cflag &= ~PARENB;
        }
        else if (parity == Parity::Even)
        {
            tty.c_cflag |= PARENB;
            tty.c_cflag &= ~PARODD;
        }
        else if (parity == Parity::Odd)
        {
            tty.c_cflag |= PARENB;
            tty.c_cflag |= PARODD;
        }

        if (stopBits == 1)
        {
            tty.c_cflag &= ~CSTOPB;
        }
        else if (stopBits == 2)
        {
            tty.c_cflag |= CSTOPB;
        }
        else
        {
            throw std::invalid_argument("Unsupported stop bits");
        }

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
};

// -------------------- NMEA Parser --------------------
class NmeaParser
{
public:
    struct DateTime
    {
        int hour;
        int minute;
        int second;
        int day;
        int month;
        int year;
    };

    DateTime parse(const std::string& sentence)
    {
        if (sentence.empty() || sentence[0] != '$')
        {
            throw std::invalid_argument("Invalid NMEA sentence");
        }

        std::vector<std::string> fields = split(sentence, ',');

        if (fields[0].find("RMC") != std::string::npos)
        {
            return parseRmc(fields);
        }
        else if (fields[0].find("ZDA") != std::string::npos)
        {
            return parseZda(fields);
        }
        else
        {
            throw std::invalid_argument("Unsupported NMEA sentence type");
        }
    }

private:
    std::vector<std::string> split(const std::string& str, char delimiter)
    {
        std::vector<std::string> tokens;
        std::stringstream ss(str);
        std::string item;

        while (std::getline(ss, item, delimiter))
        {
            tokens.push_back(item);
        }
        return tokens;
    }

    DateTime parseRmc(const std::vector<std::string>& fields)
    {
        if (fields.size() < 10)
        {
            throw std::invalid_argument("Incomplete RMC sentence");
        }

        std::string timeStr = fields[1];
        std::string dateStr = fields[9];

        DateTime dt{};
        dt.hour   = std::stoi(timeStr.substr(0, 2));
        dt.minute = std::stoi(timeStr.substr(2, 2));
        dt.second = std::stoi(timeStr.substr(4, 2));

        dt.day    = std::stoi(dateStr.substr(0, 2));
        dt.month  = std::stoi(dateStr.substr(2, 2));
        dt.year   = 2000 + std::stoi(dateStr.substr(4, 2));

        return dt;
    }

    DateTime parseZda(const std::vector<std::string>& fields)
    {
        if (fields.size() < 5)
        {
            throw std::invalid_argument("Incomplete ZDA sentence");
        }

        std::string timeStr = fields[1];
        DateTime dt{};
        dt.hour   = std::stoi(timeStr.substr(0, 2));
        dt.minute = std::stoi(timeStr.substr(2, 2));
        dt.second = std::stoi(timeStr.substr(4, 2));

        dt.day    = std::stoi(fields[2]);
        dt.month  = std::stoi(fields[3]);
        dt.year   = std::stoi(fields[4]);

        return dt;
    }
};

// -------------------- Main --------------------
int main()
{
    try
    {
        // Open UART at 115200 8N1
        Uart uart("/dev/ttyS0", B115200, 8, Parity::None, 1);
        NmeaParser parser;

        while (true)
        {
            std::string line = uart.readLine();
            try
            {
                auto dt = parser.parse(line);
                std::cout << "Time: " << dt.hour << ":" << dt.minute << ":" << dt.second
                          << " Date: " << dt.day << "-" << dt.month << "-" << dt.year << std::endl;
            }
            catch (const std::exception& ex)
            {
                // Ignore unsupported or malformed sentences
            }
        }
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
#endif
