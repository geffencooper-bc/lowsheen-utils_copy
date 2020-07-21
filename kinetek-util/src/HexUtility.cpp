#include "HexUtility.h"

using std::ifstream;
using std::ostringstream;
using std::stoi;
using std::ios;

HexUtility::HexUtility(const string &hex_file_path) 
{
    // need to init other member variables
    curr_line = "";
    is_first_8 = true;
    is_eof = false;       
    last_data_line_size = 0;
    hex_file_data_size = 0;      
    total_checksum = 0;
    start_address = 0;
    

    hex_file.open(hex_file_path);
    if(hex_file.fail())
    {
        printf("Invalid File Path\n");
        // need to quit
    }
    load_hex_file_data();
}

HexUtility::~HexUtility()
{
    printf("hex-utility destructor\n");
    hex_file.close();
}

int HexUtility::get_file_data_size(uint8_t* data_size_bytes, uint8_t num_bytes)
{
    num_to_byte_list(hex_file_data_size, data_size_bytes, num_bytes);
    return hex_file_data_size;
}

void HexUtility::get_total_cs(uint8_t* cs_bytes, uint8_t num_cs_bytes)
{
    num_to_byte_list(total_checksum, cs_bytes, num_cs_bytes);
}

int HexUtility::get_start_address(uint8_t* start_address_bytes, uint8_t num_bytes)
{
    num_to_byte_list(start_address, start_address_bytes, num_bytes);
    return start_address;
}

int HexUtility::get_next_8_bytes(uint8_t* data_bytes, uint8_t num_bytes)
{
    if(is_first_8)
    {
        getline(hex_file, curr_line);
    }
    
    hex_record_type record_type = get_record_type(curr_line);

    if(record_type == END_OF_FILE || record_type == START_LINEAR_AR)
    {
        is_eof = true;
        return -1;
    }

    while(record_type != DATA)
    {
        getline(hex_file, curr_line);
        record_type = get_record_type(curr_line);
    }

    if(get_record_data_length(curr_line) < 8)
    {
        get_record_data_bytes(curr_line, data_bytes, num_bytes); // will get the whole line by default

        // filll in this last frame with 0xFF
        int last_frame_filler_amount = 8 - get_record_data_length(curr_line);
        for(int i = get_record_data_length(curr_line); i < 8; i++)
        {
            data_bytes[i] = 0xFF;
        }

        // find the sum of the data portion
        int sum = 0;
        for(int i = 0; i < get_record_data_length(curr_line); i++)
        {
            sum += data_bytes[i];
        }
        printf("=======RETURNING LAST LINE SUM=======\n");
        getline(hex_file, curr_line);
        return sum;
    }

    else if(is_first_8)
    {
        is_first_8 = false;
        int sum = get_record_data_bytes(curr_line, data_bytes, num_bytes, 0, 8);
        return sum;
    }

    else
    {
        is_first_8 = true;
        int sum = get_record_data_bytes(curr_line, data_bytes, num_bytes, 8, 8);
        return sum;
    }
}


//-------------- helper funcs

int HexUtility::get_record_data_length(const string &hex_record)
{
    return stoi(hex_record.substr(RECORD_DATA_LENGTH_START_I, 2), 0, 16);
}

int HexUtility::get_record_address(const string &hex_record)
{
    return stoi(hex_record.substr(RECORD_ADDRESS_START_I, 4), 0, 16);
}

hex_record_type HexUtility::get_record_type(const string &hex_record)
{
    return (hex_record_type)(stoi(hex_record.substr(RECORD_TYPE_START_I,2), 0, 16));
}

int HexUtility::data_string_to_byte_list(const string &hex_data, uint8_t* data_bytes, uint8_t num_data_bytes)
{
    // the number of bytes should be at least half the number of chars in the string, "AA" --> 1 byte
    if(num_data_bytes < hex_data.size()/2)
    {
        printf("%i, %i\n", num_data_bytes, hex_data.size()/2);
        return -1;
    }
    // iterate through the string only filling in data bytes every two chars, return the sum of the bytes for checksum
    int sum_bytes = 0;
    for(int i = 0; i < hex_data.size(); i+=2)
    {
        data_bytes[i/2] = stoi(hex_data.substr(i,2), 0, 16);
        //printf("DATA[%i]: %02X", i, data_bytes[i/2]);
        sum_bytes += data_bytes[i/2];
    }
    //printf("\t\tsum: %i\n\n", sum_bytes);
    return sum_bytes;
}

int HexUtility::get_record_data_bytes(const string &hex_record, uint8_t* data_bytes, uint8_t num_data_bytes, int start, int num_bytes)
{
    // the buffer should be at least the size of the number of bytes to extract
    // otherwise it needs to be at least the size of the record length
    if( (num_data_bytes < num_bytes) || ( (num_bytes == -1) && num_data_bytes < get_record_data_length(hex_record)))
    {
        return -1;
    }
    
    start *= 2; // remember that number of chars = 2*number of bytes
    
    // get the data bytes as a string
    string data = hex_record.substr(RECORD_DATA_START_I, 2*get_record_data_length(hex_record));
    
    // if asking for too many bytes or default, then just grab the whole line (16 bytes)
    if( (num_bytes == -1) || (data.size() < 2*num_bytes) )
    {
        int sum = data_string_to_byte_list(data, data_bytes, num_data_bytes);
        return sum;
    }
    else
    {
        int sum = data_string_to_byte_list(hex_record.substr(RECORD_DATA_START_I+start, 2*num_bytes), data_bytes, num_data_bytes);
       return sum;
    }
}

int HexUtility::get_record_checksum(const string &hex_line)
{
    return stoi(hex_line.substr(RECORD_DATA_START_I + 2*get_record_data_length(hex_line), 2), 0, 16);
}

int HexUtility::load_hex_file_data()
{
    string last_data_line = ""; // save the last data line so can get its size
    uint8_t byte_list[16];       // buffer to hold next 16 data bytes in the hex file
    int line_index = 0;
    uint16_t ms_16_bits = 0;
    uint16_t ls_16_bits = 0;
    while(getline(hex_file, curr_line))
    {
        if(calc_hex_checksum(curr_line) != get_record_checksum(curr_line))
        {
            printf("BAD HEX CHECKSUM, LINE: %i", line_index);
            return -1;
        }
        // first get the start address, if extended linear then need to combine  ms and ls 16 bits
        if(line_index == 0 && get_record_type(curr_line) == EXTENDED_LINEAR_AR)
        {
            get_record_data_bytes(curr_line, byte_list, 16); // most significant 16 bits stored in bytes_list
            ms_16_bits = (byte_list[0] << 8) + byte_list[1]; // need to convert from bytes list to int, ex: [0x01, 0x10] --> 0x0110
            getline(hex_file, curr_line);
            line_index +=1;
            if(get_record_type(curr_line) == DATA)
            {
                ls_16_bits = get_record_address(curr_line);
                start_address = (ms_16_bits << 16) + ls_16_bits;
            }
        }
        else if(line_index == 0 && get_record_type(curr_line) == DATA) 
        {
            start_address = get_record_address(curr_line);
        }
        if(get_record_type(curr_line) == DATA)
        {
            hex_file_data_size += get_record_data_length(curr_line);
            total_checksum += get_record_data_bytes(curr_line, byte_list, 16);

            last_data_line = curr_line;
        }
        line_index += 1;
    }

    last_data_line_size = get_record_data_length(last_data_line);

    // reset pointer to top of file
    hex_file.clear();
    hex_file.seekg(0, ios::beg);
}

void HexUtility::num_to_byte_list(int num, uint8_t* bytes, uint8_t num_bytes)
{
    // need to convert from number to list of bytes, ex: 0x00018C1D --> [0x00, 0x01, 0x8C, 0x1D]
    for(int i = 0; i < num_bytes; i++)
    {
        bytes[i] = (num >> 8*(num_bytes-i-1)) & 0xFF;
    }
}

uint8_t HexUtility::calc_hex_checksum(const string &hex_record)
{
    int size = 4 + get_record_data_length(hex_record); // first 4 bytes are fixed --> :llaaaatt
    uint8_t record[size];
    data_string_to_byte_list(hex_record.substr(1, hex_record.size()-3), record, size);

    int sum = 0;
    for(int i = 0; i < size; i++)
    {
        sum += record[i];
    }
    int two_compl = (256 - sum) % 256;
    return two_compl;
}

