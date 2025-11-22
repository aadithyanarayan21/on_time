#include <include/navic_subsystem/navic_time_processor.h>


namespace navic
{
    navic_time_processor::navic_time_processor()
    {
        this->token_vector.clear();

        this->uart_channel_handle = std::make_unique<uart_channel>("/dev/ttyS0",
                                                                   B115200,
                                                                   8,
                                                                   Parity::None,
                                                                   1
                                                                  );
    }

    void  navic_time_processor::clear_date_time()
    {
        this->parsed_date_time.hour = 0;
        this->parsed_date_time.minute = 0;
        this->parsed_date_time.second = 0;

        this->parsed_date_time.day = 0;
        this->parsed_date_time.month = 0;
        this->parsed_date_time.year = 0;
    }

    // Parse an NMEA sentence and extract time/date if available
    DateTime& navic_time_processor::parse(const std::string& sentence)
    {
        if (sentence.empty() || sentence[0] != '$')
        {
            throw std::invalid_argument("Invalid NMEA sentence");
        }

        split(sentence, ',');

        if (token_vector[0].find("RMC") != std::string::npos)
        {
            parseRmc();
        }
        else if (token_vector[0].find("ZDA") != std::string::npos)
        {
            parseZda();
        }
        else
        {
            throw std::invalid_argument("Unsupported NMEA sentence type");
        }

        return this->parsed_date_time;
    }

     // Split helper
    void navic_time_processor::split(const std::string& str, char delimiter)
    {
        std::stringstream ss(str);
        std::string item;

        this->token_vector.clear();

        while (std::getline(ss, item, delimiter))
        {
            this->token_vector.push_back(item);
        }
    }

    // Parse RMC sentence: $GNRMC,hhmmss.sss,A,...,ddmmyy,...
    void navic_time_processor::parseRmc()
    {
        this->clear_date_time();

        //less than 10 fields, then throw exception error.
        if (this->token_vector.size() < 10)
        {
            throw std::invalid_argument("Incomplete RMC sentence");
        }

        std::string timeStr = this->token_vector[1]; // hhmmss.sss
        std::string dateStr = this->token_vector[9]; // ddmmyy

        this->parsed_date_time.hour   = std::stoi(timeStr.substr(0, 2));
        this->parsed_date_time.minute = std::stoi(timeStr.substr(2, 2));
        this->parsed_date_time.second = std::stoi(timeStr.substr(4, 2));

        this->parsed_date_time.day    = std::stoi(dateStr.substr(0, 2));
        this->parsed_date_time.month  = std::stoi(dateStr.substr(2, 2));
        this->parsed_date_time.year   = 2000 + std::stoi(dateStr.substr(4, 2)); // yy → 20yy
    }

    // Parse ZDA sentence: $GNZDA,hhmmss.sss,dd,mm,yyyy,...
    void navic_time_processor::parseZda()
    {
        this->clear_date_time();

        if (this->token_vector.size() < 5)
        {
            throw std::invalid_argument("Incomplete ZDA sentence");
        }

        std::string timeStr = this->token_vector[1]; // hhmmss.sss

        this->parsed_date_time.hour   = std::stoi(timeStr.substr(0, 2));
        this->parsed_date_time.minute = std::stoi(timeStr.substr(2, 2));
        this->parsed_date_time.second = std::stoi(timeStr.substr(4, 2));

        this->parsed_date_time.day    = std::stoi(this->token_vector[2]);
        this->parsed_date_time.month  = std::stoi(this->token_vector[3]);
        this->parsed_date_time.year   = std::stoi(this->token_vector[4]);
    }

}
