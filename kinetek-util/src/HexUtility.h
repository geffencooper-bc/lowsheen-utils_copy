//==================================================================
// Copyright 2020 Brain Corporation. All rights reserved. Brain
// Corporation proprietary and confidential.
// ALL ACCESS AND USAGE OF THIS SOURCE CODE IS STRICTLY PROHIBITED
// WITHOUT EXPRESS WRITTEN APPROVAL FROM BRAIN CORPORATION.
// Portions of this Source Code and its related modules/libraries
// may be governed by one or more third party licenses, additional
// information of which can be found at:
// https://info.braincorp.com/open-source-attributions
//==================================================================

// a utility class with helper functions to help extract data from a hex file
// Note: this class assumes that the hex file data records are 16 bytes long

// Entries in hex files (called records) follow this format

/*
    :llaaaatt[dd...dd]cc

    :          signifies the start of a record
    ll         data length field --> signifies number of bytes in data field of record
    aaaa       address field --> signifies the address of a data field
    tt         type field --> signifies the record type
    [dd...dd]  data field --> signifies the data bytes
    cc         checksum field --> signifies the two byte checksum
*/

// function details

/*
    CAN data is sent as an array of bytes, ex: {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08}
    a portion of the data array may contain a number, like the starting address, that requires multiple bytes

    ex: starting address 0x08008000 gets split up into 0x08 0x00 0x80 0x00 and placed into the data array

    some functions will require an array to be passed in to store these bytes. The according section of the data
    array can be passed in as the buffer --> ex: to get the 4 address bytes from above, you might pass in
    position two of a data array and a size of 4 so that the final array is: {0x01, 0x08, 0x00, 0x80, 0x00, 0x06, 0x07,
   0x08}
*/

#ifndef HEX_UTILITY_H
#define HEX_UTILITY_H

#include <string>
#include <fstream>
#include <stdint.h>

using std::string;
using std::ifstream;

// each record in a hex file has a type
enum hex_record_type
{
    DATA = 0,
    END_OF_FILE = 1,
    EXTENDED_SEGMENT_AR = 2,
    EXTENDED_LINEAR_AR = 4,
    START_LINEAR_AR = 5
};

// start index of fields in a hex record
enum record_indices
{
    RECORD_DATA_LENGTH_START_I = 1,
    RECORD_ADDRESS_START_I = 3,
    RECORD_TYPE_START_I = 7,
    RECORD_DATA_START_I = 9
};

// can frames have at most 8 data bytes
#define CAN_DATA_LEN 8

// assume data records in hex file are 16 bytes
#define HEX_DATA_RECORD_LEN 16

class HexUtility
{
   public:
    // opens file, then loads hex data like checksums, data size, etc
    HexUtility(const string& hex_file_path);

    // closes file
    ~HexUtility();

    // parameters: >= 4 byte array to be filled with hex file data size.
    // return: data size as an int
    int get_file_data_size(uint8_t* byte_array, uint8_t arr_size);

    // parameters: >= 4 byte array to be filled with checksum, specify if want bytes in reverse
    // return: checksum as an int
    int get_total_cs(uint8_t* byte_array, uint8_t arr_size, bool rev = false);

    // parameters: >= 4 byte array to be filled with start address
    // return: start addess as an int
    int get_start_address(uint8_t* byte_array, uint8_t arr_size);

    // parameters: >= 8 byte array to be filled with the next 8 data bytes in the hex file
    // return: sum of the 8 data bytes as an int, returns -1 if there is no more data (EOF)
    // Note: if record has less than 8 bytes, remaining bytes will be filled with 0xFF (not included in sum)
    int get_next_8_bytes(uint8_t* byte_array, uint8_t arr_size);

    // converts a number's hex representation to a list of bytes
    // parameters: number to convert, array to store byte representation
    // Note: array size determines representation (if to add extra 0x00 filler)
    // ex: 1000 in hex is 0x3E8 --> {0x03, 0xE8}
    void num_to_byte_list(int num, uint8_t* byte_array, uint8_t arr_size);

   private:
    ifstream hex_file;  // file is open for object lifetime
    string curr_line;   // file will be read line by line

    bool is_first_8;  // reading 1st 8 data bytes or next 8 data bytes in hex record
    bool is_eof;
    uint32_t hex_file_data_size;
    uint32_t total_checksum;
    uint32_t start_address;

    // Helper functions

    // first five functions extract and return certain part of hex record fed in as a string
    int get_record_data_length(const string& hex_record);  // in bytes
    int get_record_address(const string& hex_record);
    hex_record_type get_record_type(const string& hex_record);
    // fills in an array passed in with the data portion of a hex record
    // can specify number of bytes to get from a starting byte (ex: byte 2), returns sum of the bytes
    // if start and num_bytes not specified, then all data bytes in line will be grabbed
    int get_record_data_bytes(const string& hex_record,
                              uint8_t* byte_array,
                              uint8_t arr_size,
                              int start = 0,
                              int num_bytes = -1);
    int get_record_checksum(const string& hex_record);

    // converts a string of bytes "AABBCCDD" to an array of bytes {0xAA, 0xBB, 0xCC, 0xDD}
    int data_string_to_byte_list(const string& hex_data, uint8_t* byte_array, uint8_t arr_size);
    int load_hex_file_data();
    uint8_t calc_hex_checksum(const string& hex_record);
};

// special hex utility getline function that can handle dos and linux newlines, returns false when reach EOF
bool hu_getline(std::istream& file, std::string& str);
void getline_test(string file_path);
#endif