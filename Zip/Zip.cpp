// Zip.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <iomanip>
#include <map>
#include <cstdio>


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
        std::memcpy(&value, m_ptr, sizeof(std::uint16_t));
        m_ptr += sizeof(std::uint16_t);
        value = convert_to_host(value);
    }

    void parse_field(std::uint32_t& value)
    {
        std::memcpy(&value, m_ptr, sizeof(std::uint32_t));
        m_ptr += sizeof(std::uint32_t);
        value = convert_to_host(value);
    }

    std::uint32_t check_signature()
    {
        std::uint32_t value{};
        std::memcpy(&value, m_ptr, sizeof(std::uint32_t));
        value = convert_to_host(value);
        return value;
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
        std::uint32_t signature{};
        std::uint16_t version_made_by{};
        std::uint16_t version_need_to_extract{};
        Flags flags{};
        std::uint16_t compression{};
        MS_Dos_time time{};
        MS_Dos_date date{};
        std::uint32_t crc{};
        std::uint32_t compressed_size{};
        std::uint32_t uncompressed_size{};
        std::uint16_t file_name_length{};
        std::uint16_t extra_field_length{};
        std::uint16_t file_comment_length{};
        std::uint16_t disk_number{};
        std::uint16_t internal_attr{};
        std::uint32_t external_attr{};
        std::uint32_t offset_of_local_header{};
        std::string file_name{};
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
        std::memcpy(name, m_ptr, header.file_name_length);
        std::string file_name{ name };
        m_ptr += header.file_name_length;
        m_ptr += header.extra_field_length;

        m_ptr += header.compressed_size;

        return { header, file_name, {} };
    }

    Central_directory parse_central_dir()
    {
        Central_directory header{};
        parse_field(header.signature);
        parse_field(header.version_made_by);
        parse_field(header.version_need_to_extract);
        parse_field(header.flags.data);
        parse_field(header.compression);
        parse_field(header.time.data);
        parse_field(header.date.data);
        parse_field(header.crc);
        parse_field(header.compressed_size);
        parse_field(header.uncompressed_size);
        parse_field(header.file_name_length);
        parse_field(header.extra_field_length);
        parse_field(header.file_comment_length);
        parse_field(header.disk_number);
        parse_field(header.internal_attr);
        parse_field(header.external_attr);
        parse_field(header.offset_of_local_header);

        char name[200]{};
        std::memcpy(name, m_ptr, header.file_name_length);
        header.file_name = name;
        m_ptr += header.file_name_length;
        m_ptr += header.extra_field_length;
        m_ptr += header.file_comment_length;
        
        return header;
    }

    struct EndOfCentralDir
    {
        std::uint32_t signature{};
        std::uint16_t disk_number{};
        std::uint16_t disk_number2{};
        std::uint16_t total_entries{};
        std::uint16_t total_entries2{};
        std::uint32_t central_dir_size{};
        std::uint32_t starting_disk_num{};
        std::uint16_t zipfile_comment_length{};
        std::string zipfile_comment{};
    };

    EndOfCentralDir parse_end_of_central_dir()
    {
        EndOfCentralDir header{};
        parse_field(header.signature);
        parse_field(header.disk_number);
        parse_field(header.disk_number2);
        parse_field(header.total_entries);
        parse_field(header.total_entries2);
        parse_field(header.central_dir_size);
        parse_field(header.starting_disk_num);
        parse_field(header.zipfile_comment_length);

        char name[200]{};
        std::memcpy(name, m_ptr, header.zipfile_comment_length);
        header.zipfile_comment = name;

        return header;
    }

    void parse_zip(std::filesystem::path path)
    {
        auto file_size = std::filesystem::file_size(path);
        m_buffer.resize(file_size);

        std::ifstream stream{};
        stream.open(path, std::ios::binary);
        stream.read(reinterpret_cast<char*>(m_buffer.data()), file_size);
        stream.close();

        auto header_size = sizeof(Local_header);
        m_ptr = m_buffer.data();
        auto signature{ check_signature() };
        do {
            m_files.push_back(parse_zipfile());
            signature = check_signature();
        } while (signature == 0x04034b50);

        signature = check_signature();
        do {
            m_dirs.push_back(parse_central_dir());
            signature = check_signature();
        } while (signature == 0x02014b50);

        m_end_of_central_dir = parse_end_of_central_dir();

    }

    void print_attributes()
    {
        for (auto const& file : m_files)
        {
            std::cout << "Local header:\n";
            std::cout << "\tSignature: 0x" << std::hex << std::setw(8) << std::setfill('0') << file.header.signature << "\n";
            std::cout << std::dec << "\tVersion needed to extract: " << file.header.version << "\n";
            std::cout << "\tFlags: " << file.header.flags.use_data_descriptor << "\n";
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

        for (auto const& central_dir : m_dirs)
        {
            std::cout << "Central Dir\n";
            std::cout << "\tFile name: " << central_dir.file_name << "\n";
        }

        std::cout << "Number of entries in central dir: " << m_end_of_central_dir.total_entries << "\n";
    }

private:
    std::map<std::uint8_t, std::string> m_version_map{
        { 0, "MS-DOS/FAT32"},
    };

    std::vector<std::uint8_t> m_buffer{};
    std::uint8_t* m_ptr{ nullptr };
    std::vector<Zip_file> m_files{};
    std::vector<Central_directory> m_dirs{};

    EndOfCentralDir m_end_of_central_dir{};
};

}

int main()
{

    Zip::Parse p{};

    std::filesystem::path path("D:/CodeProjects/Zip/Example/package.zip");
    p.parse_zip(path);
    p.print_attributes();
}
