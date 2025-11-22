#include <include/display_subsystem/spi_channel.h>

namespace IO_CHANNEL
{
    spi_channel::spi_channel(const std::filesystem::path& in_spi_path_ref)
        : spi_fd(-1)
    {
        this->spi_open(in_spi_path_ref);
    }

    spi_channel::~spi_channel()
    {
        this->spi_close();
    }

    //opens the spi subsystem....
    //closes existing spi, if any..
    int spi_channel::spi_open(const std::filesystem::path& in_spi_path_ref)
    {
        this->spi_close();

        //open the SPI to display device... write only mode.
        spi_fd = open(in_spi_path_ref.c_str(), O_WRONLY);

        if (spi_fd < 0)
        {
            perror("open spi");
            return 1;
        }

        if (ioctl(spi_fd, SPI_IOC_WR_MODE, &SPI_MODE) < 0)
        {
            perror("SPI mode");
            return 1;
        }

        if (ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &BITS_PER_WORD) < 0)
        {
            perror("SPI bpw");
            return 1;
        }

        if (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &SPEED) < 0)
        {
            perror("SPI speed");
            return 1;
        }

        return 0;
    }

    void spi_channel::spi_close()
    {
        // Close SPI FD
        if (this->spi_fd >= 0)
        {
            ::close(this->spi_fd);
            this->spi_fd = -1;  // mark as closed
        }
    }

    void spi_channel::spi_write(const BYTE* in_data,  uint16_t in_size)
    {
        struct spi_ioc_transfer tr{};

        tr.tx_buf = (unsigned long)in_data; //set the address from which the data will be fetched
        tr.rx_buf = 0;
        tr.len = in_size; //number of bytes to be transmitted
        tr.speed_hz = SPEED;
        tr.bits_per_word = BITS_PER_WORD; //8 bits (byte) per word of data
        tr.delay_usecs = 0;

        if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr) < 0)
        {
            perror("SPI transfer");
        }
    }
}
