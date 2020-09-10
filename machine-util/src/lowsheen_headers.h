


#pragma once
#include <cstdint>

#pragma pack(push, 1)

namespace lowsheen
{

#define MAGIC_NUMBER 1296387957
typedef struct
{
    uint32_t      magic_number; //1296387957
    uint8_t       mode;         //# 2 = xt_can mode, 1 = dfu mode, 0 to restart the STM
    uint16_t      checksum;     //# set but not validated
} config_packet_t;

typedef struct
{
    uint64_t timestamp;
    uint32_t packet_id;
    uint32_t packet_reset;
    uint32_t date;
    uint32_t time;
    uint8_t log;
    uint8_t generate_coverage;
    uint16_t checksum;
} request_packet_t;

typedef struct
{
    uint32_t      header_begin_flag;        /* 0xDEADBEEF in little-endian format*/
    uint32_t      header_protocol_version;  /* header version*/
    uint32_t      git_commit_hash;          /* shortened commit hash of lowsheen*/
    uint8_t       safe_to_flash;            /* flag stating if machine is safe to flash based on encoder speed*/
    uint8_t       machine_id;               /* see machine id types listed above*/
    uint32_t      request_packet_id;        /* the packet_id of the request packet being responded to*/
    uint8_t       header_info_only;         /* human-readable flag stating that scrubber state is incomplete*/
} header_100_t;

typedef struct
{
    uint32_t      header_begin_flag;        /* 0xDEADBEEF in little-endian format*/
    uint32_t      header_protocol_version;  /* header version*/
    uint32_t      git_commit_hash;          /* shortened commit hash of lowsheen*/
    uint8_t       safe_to_flash;            /* flag stating if machine is safe to flash based on encoder speed*/
    uint8_t       machine_id;               /* see machine id types listed above*/
    uint32_t      request_packet_id;        /* the packet_id of the request packet being responded to*/
    uint8_t       header_info_only;         /* human-readable flag stating that scrubber state is incomplete*/
    uint8_t       board_version;            /* detected board version */
    uint32_t      estop_code;               /* error code causing estop */
    uint32_t      estop_call_addr;          /* address of the function that called estop */
    uint8_t       estop_exception;          /* exception # that caused the estop, 0 if not caused by exception */
    uint32_t      estop_mem_addr;           /* memory address that caused the estop, 0 if not caused by memory access */
    uint8_t       controller_type;          /* defines which controller type is attached to the system */
    uint32_t      controller_can_bitrate;   /* bitrate of CAN bus for controllers */
    uint32_t      auxiliary_can_bitrate;    /* bitrate of CAN bus for auxiliary devices */
} header_110_t;

typedef struct
{
    uint32_t      header_begin_flag;        /* 0xDEADBEEF in little-endian format*/
    uint32_t      header_protocol_version;  /* header version*/
    uint32_t      git_commit_hash;          /* shortened commit hash of lowsheen*/
    uint8_t       safe_to_flash;            /* flag stating if machine is safe to flash based on encoder speed*/
    uint8_t       machine_id;               /* see machine id types listed above*/
    uint32_t      request_packet_id;        /* the packet_id of the request packet being responded to*/
    uint8_t       header_info_only;         /* human-readable flag stating that scrubber state is incomplete*/
    uint8_t       board_version;            /* detected board version */
    uint32_t      estop_code;               /* error code causing estop */
    uint32_t      estop_call_addr;          /* address of the function that called estop */
    uint8_t       estop_exception;          /* exception # that caused the estop, 0 if not caused by exception */
    uint32_t      estop_mem_addr;           /* memory address that caused the estop, 0 if not caused by memory access */
    uint8_t       controller_type;          /* defines which controller type is attached to the system */
    uint32_t      controller_can_bitrate;   /* bitrate of CAN bus for controllers */
    uint32_t      auxiliary_can_bitrate;    /* bitrate of CAN bus for auxiliary devices */
    uint32_t      interface_version;        /* message files checksum */
} header_120_t;

// the latest defined protocol belongs here
typedef header_120_t header_t;

#pragma pack(pop)

bool interface_decode(header_t *header, uint8_t *data, int32_t data_size);
int32_t interface_request_size();
void generate_request_packet(request_packet_t *packet);

}
