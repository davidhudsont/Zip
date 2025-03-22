// Zip.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <iomanip>
#include <map>


uint16_t convert_to_host(uint16_t value) {
    return value;
}

// Function to swap the byte order of a 32-bit value
uint32_t convert_to_host(uint32_t value) {
    return value;
}

namespace Zip
{
class Parse
{
public:
    Parse()
    {

    }

    void parse_field(std::uint16_t& value)
    {
        std::memcpy(&value, ptr, sizeof(std::uint16_t));
        ptr += sizeof(std::uint16_t);
        value = convert_to_host(value);
    }

    void parse_field(std::uint32_t& value)
    {
        std::memcpy(&value, ptr, sizeof(std::uint32_t));
        ptr += sizeof(std::uint32_t);
        value = convert_to_host(value);
    }

    union Flags
    {
        struct
        {
            std::uint16_t encrypted : 1;
            std::uint16_t compresssion : 2;
            std::uint16_t use_data_descriptor : 1;
            std::uint16_t unused : 12;
        };
        std::uint16_t data;
    };


    union MS_Dos_date
    {
        struct {
            std::uint16_t day_of_month : 5;
            std::uint16_t month : 4;
            std::uint16_t year : 7;
        };
        std::uint16_t data;
    };

    union MS_Dos_time
    {
        struct {
            std::uint16_t second : 5;
            std::uint16_t minute : 6;
            std::uint16_t hour : 5;
        };
        std::uint16_t data;
    };

    struct Local_header
    {
        std::uint32_t signature{};
        std::uint16_t version{};
        Flags flags{};
        std::uint16_t compression{};
        MS_Dos_time time{};
        MS_Dos_date date{};
        std::uint32_t crc{};
        std::uint32_t compressed_size{};
        std::uint32_t uncompressed_size{};
        std::uint16_t file_name_length{};
        std::uint16_t extra_field_length{};
    };

    struct Zip_file
    {
        Local_header header{};
        std::string file_name{};
        std::vector<std::uint8_t> m_file_data{};
    };

    struct Central_directory {

    };

    Zip_file parse_zipfile()
    {
        Local_header header{};
        parse_field(header.signature);
        parse_field(header.version);
        parse_field(header.flags.data);
        parse_field(header.compression);
        parse_field(header.time.data);
        parse_field(header.date.data);
        parse_field(header.crc);
        parse_field(header.compressed_size);
        parse_field(header.uncompressed_size);
        parse_field(header.file_name_length);
        parse_field(header.extra_field_length);

        char name[200]{};
        std::memcpy(name, ptr, header.file_name_length);
        std::string file_name{ name };
        ptr += header.file_name_length;
        ptr += header.extra_field_length;

        ptr += header.compressed_size;

        return { header, file_name, {} };
    }

    void parse_zip(std::filesystem::path path)
    {
        auto file_size = std::filesystem::file_size(path);
        m_stream.open(path);
        m_buffer.resize(file_size);

        m_stream.read(reinterpret_cast<char*>(m_buffer.data()), file_size);

        auto header_size = sizeof(Local_header);
        ptr = m_buffer.data();
        m_files.push_back(parse_zipfile());
        m_files.push_back(parse_zipfile());
    }

    void print_attributes()
    {
        for (auto const& file : m_files)
        {
            std::cout << "Local header:\n";
            std::cout << "\tSignature: 0x" << std::hex << std::setw(8) << std::setfill('0') << file.header.signature << "\n";
            std::cout << std::dec << "\tVersion needed to extract: " << file.header.version << "\n";
            std::cout << "\tCompression method: " << file.header.compression << "\n";
            std::cout << "\tDate: " << file.header.date.month << "/" << file.header.date.day_of_month << "/" << file.header.date.year + 1980 << "\n";
            std::cout << "\tTime: " << file.header.time.hour << ":" << file.header.time.minute << ":" << file.header.time.second * 2 << "\n";
            std::cout << std::hex << "\tCRC: 0x" << file.header.crc << "\n";
            std::cout << std::dec << "\tCompressed Size: " << file.header.compressed_size << " Bytes\n";
            std::cout << "\tUncompressed Size: " << file.header.uncompressed_size << " Bytes\n";
            std::cout << "\tFile name length: " << file.header.file_name_length << " Bytes\n";
            std::cout << "\tExtra field length: " << file.header.extra_field_length << " Bytes\n";
            std::cout << "\tFile name: " << file.file_name << "\n";
        }
    }

private:
    std::ifstream m_stream;
    std::vector<std::uint8_t> m_buffer;
    std::uint8_t* ptr = nullptr;

    std::vector<Zip_file> m_files{};
    std::map<std::uint8_t, std::string> m_version_map{
        { 0, "MS-DOS/FAT32"},
        { 14, "VFAT"},
        { 20, "Unused"},
    };
};

}

int main()
{
    std::filesystem::path path("D:/CodeProjects/Zip/Example/example.zip");

    Zip::Parse p{};

    p.parse_zip(path);

    p.print_attributes();

}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
