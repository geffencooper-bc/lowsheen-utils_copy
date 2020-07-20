#ifndef HEX_UTIL_H
#define HEX_UTIL_H

#include <string>
#include <fstream>
#include <sstream>
#include <stdint.h>

using std::string;
using std::ifstream;
// a utility class with helper functions to help read data from a hex file
// it relies on reading from a hex file and keeping the current position

// Hex file details

// Entries in hex files follow this format
//
// :llaaaatt[dd...dd]cc
//
// :          signifies the start of a line
// ll         signifies the number of bytes in the data record
// aaaa       signifies the address of this data field
// tt         signifies the record type
// [dd...dd]  signifies the data bytes
// cc         signifies the two byte checksum

enum hex_record_type
{
    DATA = 0,
    END_OF_FILE = 1,
    EXTENDED_SEGMENT_AR = 2,
    EXTENDED_LINEAR_AR = 4,
    START_LINEAR_AR = 5
};

const uint8_t CAN_MAX_DATA_LEN = 8;
const uint8_t RECORD_DATA_LENGTH_START_I = 1;
const uint8_t RECORD_ADDRESS_START_I = 3;
const uint8_t RECORD_TYPE_START_I = 7;
const uint8_t RECORD_DATA_START_I = 9;
const uint8_t KT_CHECKSUM_SIZE = 4; // size in bytes

class HexUtility
{
    public:
    // opens file, then loads hex data like checksums, data size, etc
    HexUtility(const string &hex_file_path);

    // closes file
    ~HexUtility();

    // accessors
    int get_file_data_size(uint8_t* data_size_bytes, uint8_t num_bytes);
    
    uint8_t get_last_data_line_size()
    {
        return last_data_line_size;
    }
    
    // pass in a 4 byte buffer to be filled with cs bytes 
    void get_total_cs(uint8_t* cs_bytes, uint8_t num_cs_bytes);

    // pass in a 4 byte buffer to be filled with address bytes, also gets returned as an int
    int get_start_address(uint8_t* start_address_bytes, uint8_t num_bytes);

    // returns how many data bytes there are if < 8
    uint8_t get_next_8_bytes(uint8_t* data_bytes, uint8_t num_bytes);
    
    void num_to_byte_list(int num, uint8_t* bytes, uint8_t num_bytes);

    private:
    ifstream hex_file; // file is open for object lifetime
    string curr_line;  // file will be read line by line

    bool is_first_8;   // reading 1st 8 bytes or 2nd 8 bytes of 16 data bytes in each hex record 
    bool is_eof;       
    uint8_t last_data_line_size; // needed by IAP
    int hex_file_data_size;      
    uint32_t total_checksum;     // sum of all data bytes
    uint32_t start_address;
    
    
    // Helper functions that should not be exposed

    // first five functions extract and return certain part of hex record fed in as a string
    int get_record_data_length(const string &hex_record); // in bytes
    int get_record_address(const string &hex_record);
    hex_record_type get_record_type(const string &hex_record);
    // extracts the data bytes and fills in a buffer passed in
    int get_record_data_bytes(const string &hex_record, uint8_t* data_bytes, uint8_t num_data_bytes, int start=0, int num_bytes=-1);
    int get_record_checksum(const string &hex_record);

    // converts a string of bytes "AABBCCDD" to an array of bytes [0xAA, 0xBB, 0xCC, 0xDD]
    int data_string_to_byte_list(const string &hex_data, uint8_t* data_bytes, uint8_t num_data_bytes);
    int load_hex_file_data();    
    uint8_t calc_hex_checksum(const string &hex_record);
};

#endif