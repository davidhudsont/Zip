// Zip.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <iomanip>
#include <map>


uint16_t swap_endian(uint16_t value) {
    return (value << 8) | (value >> 8);
}

// Function to swap the byte order of a 32-bit value
uint32_t swap_endian(uint32_t value) {
    return (value << 24) | ((value << 8) & 0x00FF0000) | ((value >> 8) & 0x0000FF00) | (value >> 24);
}

// Function to swap the byte order of a 64-bit value
uint64_t swap_endian(uint64_t value) {
    return (value << 56) | ((value << 40) & 0x00FF000000000000) | ((value << 24) & 0x0000FF0000000000) | ((value << 8) & 0x000000FF00000000) |
        ((value >> 8) & 0x00000000FF000000) | ((value >> 24) & 0x0000000000FF0000) | ((value >> 40) & 0x000000000000FF00) | (value >> 56);
}

std::uint8_t* parse_field(std::uint8_t* ptr, std::uint16_t & value)
{
    std::memcpy(&value, ptr, sizeof(std::uint16_t));
    return ptr + sizeof(std::uint16_t);
}

std::uint8_t* parse_field(std::uint8_t* ptr, std::uint32_t& value)
{
    std::memcpy(&value, ptr, sizeof(std::uint32_t));
    return ptr + sizeof(std::uint32_t);
}





namespace Zip
{
class Parse
{
public:
    Parse()
    {

    }

    struct Flags
    {
        union
        {
            std::uint16_t encyped : 1;
            std::uint16_t compresssion : 2;
            std::uint16_t use_data_descriptor : 1;
            std::uint16_t unused : 12;
            std::uint16_t flags;
        };
    };


    struct MS_Dos_date {
        union
        {
            std::uint16_t day_of_month : 5;
            std::uint16_t month : 4;
            std::uint16_t year : 7;
            std::uint16_t date;
        };
    };

    struct MS_Dos_time {
        union
        {
            std::uint16_t second : 5;
            std::uint16_t minute : 6;
            std::uint16_t hour : 5;
            std::uint16_t time;
        };
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

    void parse_zip(std::filesystem::path path)
    {
        auto file_size = std::filesystem::file_size(path);
        m_stream.open(path);
        m_buffer.resize(file_size);

        m_stream.read(reinterpret_cast<char*>(m_buffer.data()), file_size);

        auto header_size = sizeof(Local_header);
        std::uint8_t* ptr = m_buffer.data();

        ptr = parse_field(ptr, m_header.signature);
        ptr = parse_field(ptr, m_header.version);
        ptr = parse_field(ptr, m_header.flags.flags);
        ptr = parse_field(ptr, m_header.compression);
        ptr = parse_field(ptr, m_header.time.time);
        ptr = parse_field(ptr, m_header.date.date);
        ptr = parse_field(ptr, m_header.crc);
        ptr = parse_field(ptr, m_header.compressed_size);
        ptr = parse_field(ptr, m_header.uncompressed_size);
        ptr = parse_field(ptr, m_header.file_name_length);
        ptr = parse_field(ptr, m_header.extra_field_length);

        char name[200]{};
        std::memcpy(name, ptr, m_header.file_name_length);
        file_name = name;
        ptr += m_header.file_name_length;
        ptr += m_header.extra_field_length;
    }

    void print_attributes()
    {
        std::cout << "Local header:\n";
        std::cout << "\tSignature: 0x" << std::hex << std::setw(8) << std::setfill('0') << m_header.signature << "\n";
        std::cout << std::dec << "\tVersion needed to extract: " << m_header.version << "\n";
        std::cout << "\tCompression method: " << m_header.compression << "\n";
        std::cout << "\tDate: " << m_header.date.month << "/" << m_header.date.day_of_month << "/" << m_header.date.year + 1980 << "\n";
        std::cout << "\tTime: " << m_header.time.hour << ":" << m_header.time.minute << ":" << m_header.time.second * 2 << "\n";
        std::cout << std::hex << "\tCRC: 0x" << m_header.crc << "\n";
        std::cout << std::dec << "\tCompressed Size: " << m_header.compressed_size << " Bytes\n";
        std::cout << "\tUncompressed Size: " << m_header.uncompressed_size << " Bytes\n";
        std::cout << "\tFile name length: " << m_header.file_name_length << " Bytes\n";
        std::cout << "\tExtra field length: " << m_header.extra_field_length << " Bytes\n";
        std::cout << "\tFile name: " << file_name << "\n";
    }

private:
    std::ifstream m_stream;
    std::vector<std::uint8_t> m_buffer;
    Local_header m_header{};
    std::string file_name{};
    std::map<std::uint8_t, std::string> m_version_map{
        { 0, "MS-DOS/FAT32"},
        { 14, "VFAT"},
        { 20, "Unused"},
    };
};

}

int main()
{
    std::filesystem::path path("D:/CodeProjects/Zip/Example/example1.zip");

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
