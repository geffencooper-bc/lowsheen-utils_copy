//==================================================================
// Copyright 2020 Brain Corporation. All rights reserved. Brain
// Corporation proprietary and confidential.
// ALL ACCESS AND USAGE OF THIS SOURCE CODE IS STRICTLY PROHIBITED
// WITHOUT EXPRESS WRITTEN APPROVAL FROM BRAIN CORPORATION.
// Portions of this Source Code and its related modules/libraries
// may be governed by one or more third party licenses, additional
// information of which can be found at:
// https://info.braincorp.com/open-source-attributions


// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//==================================================================


#include "HexUtility.h"

using std::stoi;
using std::ios;

// opens file and loads hex data like checksums, data size, etc
HexUtility::HexUtility()
{
    // init member variables
    curr_line = "";
    is_first_8 = true;
    is_eof = false;
    hex_file_data_size = 0;
    total_checksum = 0;
    start_address = 0;
}

// close file, nothing to deallocate
HexUtility::~HexUtility()
{
    if (hex_file.is_open())
    {
        hex_file.close();
    }
}

// get the number of data bytes in the hex file
int HexUtility::get_file_data_size(uint8_t* byte_array, uint8_t arr_size)
{
    if (arr_size < 4)
    {
        DEBUG_PRINTF("ERROR: Array size is not big enough\n");
        exit(EXIT_FAILURE);
    }
    // store file data size as array of 4 bytes--> Kinetek format
    num_to_byte_list(hex_file_data_size, byte_array, 4);
    return hex_file_data_size;
}

// get the checksum for entire hex file
int HexUtility::get_total_cs(uint8_t* byte_array, uint8_t arr_size, bool rev)
{
    if (arr_size < 4)
    {
        DEBUG_PRINTF("ERROR: Array size is not big enough\n");
        exit(EXIT_FAILURE);
    }
    // kinetek expects total checksum in reverse (little endian) after sending hex file
    if (rev)
    {
        // store checksum as array of 4 bytes--> Kinetek format
        num_to_byte_list(__builtin_bswap32(total_checksum), byte_array, 4);
    }
    // kinetek expects total checksum in big endian when sending init packets
    else
    {
        num_to_byte_list(total_checksum, byte_array, 4);
    }
    return total_checksum;
}

// get the start address for the data
int HexUtility::get_start_address(uint8_t* byte_array, uint8_t arr_size)
{
    if (arr_size < 4)
    {
        DEBUG_PRINTF("ERROR: Array size is not big enough\n");
        exit(EXIT_FAILURE);
    }
    // store the start address as an array 4 of bytes--> Kinetek format
    num_to_byte_list(start_address, byte_array, 4);
    return start_address;
}

// get the next can frame data from the hex file
int HexUtility::get_next_8_bytes(uint8_t* byte_array, uint8_t arr_size)
{
    if (!hex_file.is_open())
    {
        hex_file.open(hex_file_path);
    }
    if (arr_size < 8)
    {
        DEBUG_PRINTF("ERROR: Array size is not big enough\n");
        exit(EXIT_FAILURE);
    }
    // get next line because start of new record
    if (is_first_8)
    {
        hu_getline(hex_file, curr_line);
    }

    hex_record_type record_type = get_record_type(curr_line);

    // if hit these record types then have read all data
    if (record_type == END_OF_FILE || record_type == START_LINEAR_AR)
    {
        is_eof = true;
        if (hex_file.is_open())
        {
            hex_file.close();
        }
        return -1;
    }

    // get the next data line
    while (record_type != DATA)
    {
        hu_getline(hex_file, curr_line);
        record_type = get_record_type(curr_line);
    }

    // grab the first 8 data bytes in a hex record
    if (is_first_8)
    {
        is_first_8 = false;
        // when record has <= 8 data bytes, fill remainder with 0xFF and go to next line
        if (get_record_data_length(curr_line) <= CAN_DATA_LEN)
        {
            // if number of bytes is not specified, it will get all the data bytes in the record
            int sum = get_record_data_bytes(curr_line, byte_array, arr_size);

            // fill in remaining spots with 0xFF
            int filler_amount = CAN_DATA_LEN - get_record_data_length(curr_line);
            for (int i = get_record_data_length(curr_line); i < CAN_DATA_LEN; i++)
            {
                byte_array[i] = 0xFF;
            }
            // get the next line so the next time the function gets called it will reach eof and not try to grab the
            // second 8 bytes
            hu_getline(hex_file, curr_line);
            return sum;
        }
        // record with > 8 bytes
        else
        {
            int sum = get_record_data_bytes(curr_line, byte_array, arr_size, 0, CAN_DATA_LEN);
            return sum;
        }
    }

    // grab the first 8 data bytes in a hex record
    else
    {
        is_first_8 = true;
        // when send half of record has <= 8 data bytes, fill remainder with 0xFF and go to next line
        if (get_record_data_length(curr_line) - CAN_DATA_LEN < CAN_DATA_LEN)
        {
            // grab second half of data bytes (byte 8 --> end)
            int sum = get_record_data_bytes(curr_line, byte_array, arr_size, HEX_DATA_RECORD_LEN / 2,
                                            get_record_data_length(curr_line) - CAN_DATA_LEN);

            // fill in remaining spots with 0xFF
            for (int i = get_record_data_length(curr_line) - CAN_DATA_LEN; i < CAN_DATA_LEN; i++)
            {
                byte_array[i] = 0xFF;
            }
            return sum;
        }
        // record with 16 bytes
        else
        {
            int sum = get_record_data_bytes(curr_line, byte_array, arr_size, HEX_DATA_RECORD_LEN / 2, CAN_DATA_LEN);
            return sum;
        }
    }
}

void HexUtility::clear()
{
    // init member variables
    curr_line = "";
    is_first_8 = true;
    is_eof = false;
    hex_file_data_size = 0;
    total_checksum = 0;
    start_address = 0;
}

// -------------- helper funcs --------------------

// convert field from string to int, interpret as base 16
int HexUtility::get_record_data_length(const string& hex_record)
{
    return stoi(hex_record.substr(RECORD_DATA_LENGTH_START_I, 2), 0, 16);
}

// convert field from string to int, interpret as base 16
int HexUtility::get_record_address(const string& hex_record)
{
    return stoi(hex_record.substr(RECORD_ADDRESS_START_I, 4), 0, 16);
}

// convert field from string to int, interpret as base 16
hex_record_type HexUtility::get_record_type(const string& hex_record)
{
    return (hex_record_type)(stoi(hex_record.substr(RECORD_TYPE_START_I, 2), 0, 16));
}

// convert a data field from a string to a list of bytes
int HexUtility::data_string_to_byte_list(const string& hex_data, uint8_t* byte_array, uint8_t arr_size)
{
    // the number of bytes should be at least half the number of chars in the string, 2 chars "AA" --> 1 byte
    if (arr_size < hex_data.size() / 2)
    {
        DEBUG_PRINTF("ERROR: Array size is not big enough\n");
        exit(EXIT_FAILURE);
    }
    // iterate through the string only filling in data bytes every two chars, return the sum of the bytes for checksum
    int sum_bytes = 0;
    for (int i = 0; i < hex_data.size(); i += 2)
    {
        byte_array[i / 2] = stoi(hex_data.substr(i, 2), 0, 16);
        sum_bytes += byte_array[i / 2];
    }
    return sum_bytes;
}

// convert the data field given a hex record as a string to a list of bytes, returns the sum
int HexUtility::get_record_data_bytes(const string& hex_record,
                                      uint8_t* byte_array,
                                      uint8_t arr_size,
                                      int start,
                                      int num_bytes)
{
    // the array should be at least the size of the number of bytes to extract
    // otherwise it needs to be at least the size of the record length
    if ((arr_size < num_bytes) || ((num_bytes == -1) && arr_size < get_record_data_length(hex_record)))
    {
        DEBUG_PRINTF("ERROR: Array size is not big enough\n");
        exit(EXIT_FAILURE);
    }

    start *= 2;  // remember that number of chars = 2*number of bytes

    // get the entire data field as a string
    string data_str = hex_record.substr(RECORD_DATA_START_I, 2 * get_record_data_length(hex_record));

    // if asking for too many bytes or default, then just grab the whole line
    if ((num_bytes == -1) || (data_str.size() < 2 * num_bytes))
    {
        int sum = data_string_to_byte_list(data_str, byte_array, arr_size);
        return sum;
    }
    else
    {
        int sum = data_string_to_byte_list(hex_record.substr(RECORD_DATA_START_I + start, 2 * num_bytes), byte_array,
                                           arr_size);
        return sum;
    }
}

// convert field from string to int, interpret as base 16
int HexUtility::get_record_checksum(const string& hex_line)
{
    return stoi(hex_line.substr(RECORD_DATA_START_I + 2 * get_record_data_length(hex_line), 2), 0, 16);
}

// read through entire hex file and grab neccessary information
int HexUtility::load_hex_file_data(const string& hex_file_path)
{
    this->hex_file_path = hex_file_path;

    // try to open the hex file and load the data
    if (hex_file.is_open())
    {
        DEBUG_PRINTF("ERROR: trying to reopen hex file\n");
        exit(EXIT_FAILURE);
    }
    hex_file.open(hex_file_path);
    if (hex_file.fail())
    {
        DEBUG_PRINTF("ERROR: Invalid File Path%s\n", hex_file_path.c_str());
        exit(EXIT_FAILURE);
    }

    // buffer to hold next 16 data bytes in the hex file
    uint8_t byte_list[HEX_DATA_RECORD_LEN];
    int line_index = 0;

    // used for an address stored using extended linear address
    uint16_t ms_16_bits = 0;
    uint16_t ls_16_bits = 0;

    // go through entire hex file line by line
    while (hu_getline(hex_file, curr_line))
    {
        // check to make sure all record checksums are valid (is hex file corrupt)
        if (calc_hex_checksum(curr_line) != get_record_checksum(curr_line))
        {
            DEBUG_PRINTF("ERROR: bad hex checksum line: %i", line_index);
            exit(EXIT_FAILURE);
        }
        // get the start address, if using extended linear then need to combine ms and ls 16 bits
        if (line_index == 0 && get_record_type(curr_line) == EXTENDED_LINEAR_AR)
        {
            // most significant 16 bits stored in the data portion of extended linear adress record
            get_record_data_bytes(curr_line, byte_list, sizeof(byte_list));

            // need to convert from bytes list to int, ex: [0x01, 0x10] --> 0x0110
            ms_16_bits = (byte_list[0] << 8) + byte_list[1];

            hu_getline(hex_file, curr_line);
            line_index += 1;
            if (get_record_type(curr_line) == DATA)
            {
                ls_16_bits = get_record_address(curr_line);
                start_address = (ms_16_bits << 16) + ls_16_bits;
            }
        }
        // if not extended linear address then start address is first data line address
        else if (line_index == 0 && get_record_type(curr_line) == DATA)
        {
            start_address = get_record_address(curr_line);
        }

        // count number of data bytes
        if (get_record_type(curr_line) == DATA)
        {
            hex_file_data_size += get_record_data_length(curr_line);
            total_checksum += get_record_data_bytes(curr_line, byte_list, sizeof(byte_list));
        }
        line_index += 1;
    }

    // close the file, reopen later when needed
    if (hex_file.is_open())
    {
        hex_file.close();
    }
}

void HexUtility::num_to_byte_list(int num, uint8_t* byte_array, uint8_t arr_size)
{
    // need to convert from number to list of bytes, ex: 0x00018C1D --> [0x00, 0x01, 0x8C, 0x1D]
    for (int i = 0; i < arr_size; i++)
    {
        byte_array[i] = (num >> (8 * (arr_size - i - 1))) & 0xFF;
    }
}

// calculates the checksum of a hex record using the standard formula
uint8_t HexUtility::calc_hex_checksum(const string& hex_record)
{
    int size = 4 + get_record_data_length(hex_record);  // first 4 bytes are fixed --> :llaaaatt
    uint8_t record[size];
    data_string_to_byte_list(hex_record.substr(1, hex_record.size() - 3), record, size);

    int sum = 0;
    for (int i = 0; i < size; i++)
    {
        sum += record[i];
    }
    int two_compl = (256 - sum) % 256;
    return two_compl;
}

// get line function that handles dos and linux newlines, returns false when reach eof
bool hu_getline(std::istream& file, std::string& str)
{
    str.clear();
    while (true)
    {
        int c = file.get();
        switch (c)
        {
            case '\n':
            {
                if (!str.empty())
                {
                    return true;
                }
                break;
            }
            case '\r':
            {
                break;
            }
            case EOF:
            {
                // if the file does not end with a newline then don't want to return false till next call
                if (str.empty())
                {
                    return false;
                }
                return true;
            }
            default:
            {
                str += (char)c;
                break;
            }
        }
    }
}

void getline_test(string file_path)
{
    ifstream file;
    file.open(file_path);
    if (file.fail())
    {
        DEBUG_PRINTF("ERROR: Invalid File Path%s\n", file_path.c_str());
        exit(EXIT_FAILURE);
    }

    string line = "";
    while (hu_getline(file, line))
    {
        DEBUG_PRINTF("%s", line.c_str());
    }
    if (file.is_open())
    {
        file.close();
    }
}