

#include "lowsheen_headers.h"
#include <cstring>
#include <chrono>
#include <ctime>
#include <zlib.h>

static const uint8_t PROTOCOL_VERSION_1_0_0 = 100;
static const uint8_t PROTOCOL_VERSION_1_1_0 = 110;
static const uint8_t PROTOCOL_VERSION_1_2_0 = 120;
static const uint32_t HEADER_BEGIN_FLAG =  0xDEADBEEF;

static const int32_t PROTOCOL_MAX_SIZE = 2 * 1024;  // asume interface request never bigger than this

static const uint8_t TR_HT_SHIFT = 20;  // Hour tens in BCD format shift
static const uint8_t TR_HT_MASK = 0x3;   // Hour tens in BCD format mask
static const uint8_t TR_HU_SHIFT = 16;   // Hour units in BCD format shift
static const uint8_t TR_HU_MASK = 0xf;   // Hour units in BCD format mask
static const uint8_t TR_MNT_SHIFT = 12;  // Minute tens in BCD format shift
static const uint8_t TR_MNT_MASK = 0x7;  // Minute tens in BCD format mask
static const uint8_t TR_MNU_SHIFT = 8;  // Minute units in BCD format shift
static const uint8_t TR_MNU_MASK = 0xf;  // Minute units in BCD format mask
static const uint8_t TR_ST_SHIFT = 4;    // Second tens in BCD format shift
static const uint8_t TR_ST_MASK = 0x7;   // Second tens in BCD format mask
static const uint8_t TR_SU_SHIFT = 0;    // Second units in BCD format shift
static const uint8_t TR_SU_MASK = 0xf;   // Second units in BCD format mask

static const uint8_t DR_YT_SHIFT = 20;  // Year tens in BCD format shift
static const uint8_t DR_YT_MASK = 0xf;   // Year tens in BCD format mask
static const uint8_t DR_YU_SHIFT = 16;   // Year units in BCD format shift
static const uint8_t DR_YU_MASK = 0xf;   // Year units in BCD format mask
static const uint8_t DR_WDU_SHIFT = 13;  // Weekday units shift
static const uint8_t DR_WDU_MASK = 0x7;  // Weekday units mask
static const uint8_t DR_MT_MASK = 0x01;  // Month tens in BCD format mask
static const uint8_t DR_MT_SHIFT = 12;   // Month tens in BCD format mask
static const uint8_t DR_MU_SHIFT = 8;    // Month units in BCD format shift
static const uint8_t DR_MU_MASK = 0xf;   // Month units in BCD format mask
static const uint8_t DR_DT_SHIFT = 4;    // Date tens in BCD format shift
static const uint8_t DR_DT_MASK = 0x3;   // Date tens in BCD format mask
static const uint8_t DR_DU_SHIFT = 0;    // Date units in BCD format shift
static const uint8_t DR_DU_MASK = 0xf;   // Date units in BCD format mask

static uint32_t get_bcd_date(const struct tm *timer)
{
    uint32_t bcd_date = 0;
 
    bcd_date |= (((timer->tm_year % 100)/10 & DR_YT_MASK) << DR_YT_SHIFT) | ((timer->tm_year % 10 & DR_YU_MASK) << DR_YU_SHIFT);
    bcd_date |= ((timer->tm_mon/10 & DR_MT_MASK) << DR_MT_SHIFT) | ((timer->tm_mon % 10 & DR_MU_MASK) << DR_MU_SHIFT);
    bcd_date |= ((timer->tm_wday/10 & DR_DT_MASK) << DR_DT_SHIFT) | ((timer->tm_wday % 10 & DR_DU_MASK) << DR_DU_SHIFT);

    return bcd_date;
}

static uint32_t get_bcd_time(const struct tm *dt)
{
    uint32_t bcd_time = 0;

    bcd_time |= ((dt->tm_hour/10 & TR_HT_MASK) << TR_HT_SHIFT) | ((dt->tm_hour % 10 & TR_HU_MASK) << TR_HU_SHIFT);
    bcd_time |= ((dt->tm_min/10 & TR_MNT_MASK) << TR_MNT_SHIFT) | ((dt->tm_min % 10 & TR_MNU_MASK) << TR_MNU_SHIFT);
    bcd_time |= ((dt->tm_sec/10 & TR_ST_MASK) << TR_ST_SHIFT) | ((dt->tm_sec % 10 & TR_SU_MASK) << TR_SU_SHIFT);

    return bcd_time;
}

namespace lowsheen
{
    
bool interface_decode(header_t *header, uint8_t *data, int32_t data_size)
{
    header_100_t *h;

    // the first defined protocol is 100, all packets must handle this size
    if(data_size < sizeof(header_100_t))
    {
        return false;
    }

    h = (header_100_t *)data;

    if(h->header_begin_flag != HEADER_BEGIN_FLAG)
    {
        return false;
    }

    memset(header, 0, sizeof(header_t));
    
    switch(h->header_protocol_version)
    {
        case PROTOCOL_VERSION_1_0_0:
            memcpy(header, data, sizeof(header_100_t));
            return true;
        case PROTOCOL_VERSION_1_1_0:
            memcpy(header, data, sizeof(header_110_t));
            return true;
        case PROTOCOL_VERSION_1_2_0:
            memcpy(header, data, sizeof(header_120_t));
            return true;
        default:
            return false;
    }    
}

int32_t interface_request_size()
{
    return PROTOCOL_MAX_SIZE;
}

void generate_request_packet(request_packet_t *packet)
{
    std::chrono::milliseconds ms = std::chrono::duration_cast< std::chrono::milliseconds >(
        std::chrono::system_clock::now().time_since_epoch());

    time_t timer;
    struct tm *datetime;

    time(&timer);
    datetime = gmtime (&timer);

    uLong crc = crc32(0L, Z_NULL, 0);   //obtain initial crc 

    packet->timestamp = ms.count();
    packet->packet_id = 0;
    packet->packet_reset = 1;
    packet->date = get_bcd_date(datetime);
    packet->time = get_bcd_time(datetime);
    packet->log = 0;
    packet->generate_coverage = 0;
    packet->checksum = (uint16_t)crc32(crc, (uint8_t *)packet, sizeof(request_packet_t) - 2);
}

}
