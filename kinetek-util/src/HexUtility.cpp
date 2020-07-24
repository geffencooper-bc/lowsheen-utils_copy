#include "HexUtility.h"

// #define PRINT_LOG

using std::stoi;
using std::ios;

HexUtility::HexUtility(const string &hex_file_path) 
{
    curr_line = "";
    is_first_8 = true;
    is_eof = false;       
    hex_file_data_size = 0;      
    total_checksum = 0;
    start_address = 0;
    
    hex_file.open(hex_file_path);
    if(hex_file.fail())
    {
        printf("Invalid File Path\n");
        exit(EXIT_FAILURE);
    }
    load_hex_file_data();
}

HexUtility::~HexUtility()
{
    hex_file.close();
}

int HexUtility::get_file_data_size(uint8_t* byte_array, uint8_t arr_size)
{
    if(arr_size < 4)
    {
        printf("Error: Array size is not big enough\n");
        exit(EXIT_FAILURE);
    }
    num_to_byte_list(hex_file_data_size, byte_array, arr_size);
    return hex_file_data_size;
}

void HexUtility::get_total_cs(uint8_t* byte_array, uint8_t arr_size)
{
    if(arr_size < 4)
    {
        printf("Error: Array size is not big enough\n");
        exit(EXIT_FAILURE);
    }
    num_to_byte_list(total_checksum, byte_array, arr_size);
}

int HexUtility::get_start_address(uint8_t* byte_array, uint8_t arr_size)
{
    if(arr_size < 4)
    {
        printf("Error: Array size is not big enough\n");
        exit(EXIT_FAILURE);
    }
    num_to_byte_list(start_address, byte_array, arr_size);
    return start_address;
}

int HexUtility::get_next_8_bytes(uint8_t* byte_array, uint8_t arr_size)
{
    if(arr_size < 8)
    {
        printf("Error: Array size is not big enough\n");
        exit(EXIT_FAILURE);
    }
    // get next line because start of new record
    if(is_first_8)
    {
        getline(hex_file, curr_line);
    }
    
    hex_record_type record_type = get_record_type(curr_line);

    if(record_type == END_OF_FILE || record_type == START_LINEAR_AR)
    {
        #ifdef PRINT_LOG
        printf("Last Line in hex file\n");
        #endif
        is_eof = true;
        return -1;
    }

    // get the next data line
    while(record_type != DATA)
    {
        getline(hex_file, curr_line);
        record_type = get_record_type(curr_line);
    }

    if(is_first_8)
    {   
        is_first_8 = false;
        // when record has less than 8 data bytes, it is the end of the file and needs to be filled with 0xFF
        if(get_record_data_length(curr_line) < CAN_DATA_LEN)
        {
            // if the number of bytes to get is not specified, it will get all the data bytes based on the hex record
            int sum = get_record_data_bytes(curr_line, byte_array, arr_size);

            // fill in with 0xFF
            int filler_amount = CAN_DATA_LEN - get_record_data_length(curr_line);
            for(int i = get_record_data_length(curr_line); i < CAN_DATA_LEN; i++)
            {
                byte_array[i] = 0xFF;
            }
            // get the next line so the next time the function gets called it will reach eof and not try to grab the second 8 bytes
            getline(hex_file, curr_line); 
            return sum;
        }
        else
        {
            int sum = get_record_data_bytes(curr_line, byte_array, arr_size, 0, CAN_DATA_LEN);
            return sum;
        }
    }

    else
    {
        is_first_8 = true;
        // when the second half of the data field is less than 8 bytes, it needs to be filled with 0xFF
        if(get_record_data_length(curr_line) - CAN_DATA_LEN < CAN_DATA_LEN) 
        {
            int sum = get_record_data_bytes(curr_line, byte_array, arr_size, HEX_DATA_RECORD_LEN/2, get_record_data_length(curr_line) - CAN_DATA_LEN);
            int last_frame_filler_amount = HEX_DATA_RECORD_LEN - get_record_data_length(curr_line);
            for(int i = get_record_data_length(curr_line)-CAN_DATA_LEN; i < CAN_DATA_LEN; i++)
            {
                byte_array[i] = 0xFF;
            }
        }
        else
        {
            int sum = get_record_data_bytes(curr_line, byte_array, arr_size, HEX_DATA_RECORD_LEN/2, CAN_DATA_LEN);
            return sum;
        }
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

int HexUtility::data_string_to_byte_list(const string &hex_data, uint8_t* byte_array, uint8_t arr_size)
{
    // the number of bytes should be at least half the number of chars in the string, "AA" --> 1 byte
    if(arr_size < hex_data.size()/2)
    {
        printf("Error: Array size is not big enough\n");
        exit(EXIT_FAILURE);
    }
    // iterate through the string only filling in data bytes every two chars, return the sum of the bytes for checksum
    int sum_bytes = 0;
    for(int i = 0; i < hex_data.size(); i+=2)
    {
        byte_array[i/2] = stoi(hex_data.substr(i,2), 0, 16);
        sum_bytes += byte_array[i/2];
    }
    return sum_bytes;
}

int HexUtility::get_record_data_bytes(const string &hex_record, uint8_t* byte_array, uint8_t arr_size, int start, int num_bytes)
{
    // the array should be at least the size of the number of bytes to extract
    // otherwise it needs to be at least the size of the record length
    if( (arr_size < num_bytes) || ( (num_bytes == -1) && arr_size < get_record_data_length(hex_record)))
    {
        printf("Error: Array size is not big enough\n");
        exit(EXIT_FAILURE);
    }
    
    start *= 2; // remember that number of chars = 2*number of bytes
    
    // get the entire data field as a string
    string data_str = hex_record.substr(RECORD_DATA_START_I, 2*get_record_data_length(hex_record));
    
    // if asking for too many bytes or default, then just grab the whole line
    if( (num_bytes == -1) || (data_str.size() < 2*num_bytes) )
    {
        int sum = data_string_to_byte_list(data_str, byte_array, arr_size);
        return sum;
    }
    else
    {
        int sum = data_string_to_byte_list(hex_record.substr(RECORD_DATA_START_I+start, 2*num_bytes), byte_array, arr_size);
        return sum;
    }
}

int HexUtility::get_record_checksum(const string &hex_line)
{
    return stoi(hex_line.substr(RECORD_DATA_START_I + 2*get_record_data_length(hex_line), 2), 0, 16);
}

int HexUtility::load_hex_file_data()
{
    uint8_t byte_list[HEX_DATA_RECORD_LEN]; // buffer to hold next 16 data bytes in the hex file
    int line_index = 0;
    uint16_t ms_16_bits = 0;
    uint16_t ls_16_bits = 0;

    // go through entire hex file line by line
    while(getline(hex_file, curr_line)) 
    {
        // check to make sure all record checksums are valid
        if(calc_hex_checksum(curr_line) != get_record_checksum(curr_line))
        {
            printf("BAD HEX CHECKSUM, LINE: %i", line_index);
            exit(EXIT_FAILURE);
        }
        // get the start address, if extended linear then need to combine  ms and ls 16 bits
        if(line_index == 0 && get_record_type(curr_line) == EXTENDED_LINEAR_AR)
        {
            // most significant 16 bits stored in bytes_list because extended linear adress record format
            get_record_data_bytes(curr_line, byte_list, sizeof(byte_list)); 

            // need to convert from bytes list to int, ex: [0x01, 0x10] --> 0x0110
            ms_16_bits = (byte_list[0] << 8) + byte_list[1]; 

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
            total_checksum += get_record_data_bytes(curr_line, byte_list, sizeof(byte_list));
        }
        line_index += 1;
    }

    // reset file stream pointer to top of file
    hex_file.clear();
    hex_file.seekg(0, ios::beg);
}

void HexUtility::num_to_byte_list(int num, uint8_t* byte_array, uint8_t arr_size)
{
    // need to convert from number to list of bytes, ex: 0x00018C1D --> [0x00, 0x01, 0x8C, 0x1D]
    for(int i = 0; i < arr_size; i++)
    {
        byte_array[i] = (num >> 8*(arr_size-i-1)) & 0xFF;
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

