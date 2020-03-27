from __future__ import print_function

import can 
import struct

sdo_msgs = {}

sdo_msgs[0xC6] = "SDO Block Download Initiate Request (with CRC, Size Indicator)"
sdo_msgs[0xC4] = "SDO Block Download Initiate Request (with CRC)"
sdo_msgs[0xC2] = "SDO Block Download Initiate Request (with Size Indicator)"
sdo_msgs[0xA4] = "SDO Block Download Initiate Response"
sdo_msgs[0xA2] = "SDO Block Download Response"
sdo_msgs[0xC1] = "SDO Block Download End Request (8 bytes)"
sdo_msgs[0xC5] = "SDO Block Download End Request (7 bytes)"
sdo_msgs[0xC9] = "SDO Block Download End Request (6 bytes)"
sdo_msgs[0xCD] = "SDO Block Download End Request (5 bytes)"
sdo_msgs[0xD1] = "SDO Block Download End Request (4 bytes)"
sdo_msgs[0xD5] = "SDO Block Download End Request (3 bytes)"
sdo_msgs[0xD9] = "SDO Block Download End Request (2 bytes)"
sdo_msgs[0xDD] = "SDO Block Download End Request (1 bytes)"
sdo_msgs[0xA1] = "SDO Block Download End Response"
sdo_msgs[0x80] = "SDO Abort"
sdo_msgs[0x40] = "SDO Expedite Upload Request"
sdo_msgs[0x43] = "SDO Expedite Upload Response (4 bytes)"
sdo_msgs[0x45] = "SDO Expedite Upload Response (3 bytes)"
sdo_msgs[0x47] = "SDO Expedite Upload Response (2 bytes)"
sdo_msgs[0x4F] = "SDO Expedite Upload Response (1 bytes)"
sdo_msgs[0x23] = "SDO Expedite Download Request (4 bytes)"
sdo_msgs[0x27] = "SDO Expedite Download Request (3 bytes)"
sdo_msgs[0x2B] = "SDO Expedite Download Request (2 bytes)"
sdo_msgs[0x2F] = "SDO Expedite Download Request (1 bytes)"
sdo_msgs[0x60] = "SDO Expedite Download Response"

a = [0x1, 0x03]

object_dictionary = {}
object_dictionary[0x1000] = 0x0001
object_dictionary[0x1001] = 0x0000
object_dictionary[0x1018] = 0x0002
object_dictionary[0x1F56] = 0xBEEF
object_dictionary[0x1F57] = 0x0003
object_dictionary[0x1F50] = 0x0000
object_dictionary[0x1F51] = 0x0001

crc16_ccitt_table = [
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7, \
    0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF, \
    0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6, \
    0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE, \
    0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485, \
    0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D, \
    0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4, \
    0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC, \
    0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823, \
    0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B, \
    0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12, \
    0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A, \
    0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41, \
    0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49, \
    0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70, \
    0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78, \
    0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F, \
    0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067, \
    0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E, \
    0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256, \
    0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D, \
    0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405, \
    0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C, \
    0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634, \
    0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB, \
    0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3, \
    0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A, \
    0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92, \
    0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9, \
    0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1, \
    0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8, \
    0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0 \
]
 
def crc16_ccitt( block, crc):    
    for i in range(0, len(block)):
        tmp =  ((crc >> 8) ^ block[i]) & 0xFF
        crc = ( ((crc << 8)) ^ crc16_ccitt_table[tmp]) & 0xFFFF
     #   print("DATA: {:02X} CRC: {:04X} TMP: {:02X}".format(block[i], crc, tmp))

    return crc  

def print_msg(prepend, msg):
    sdo_str = sdo_msgs.get(msg.data[0], "Unknown: " + hex(msg.data[0]))

    print(prepend + sdo_str)    


def sdo_send(msg):

    if(msg.dlc != 8):
        return

    print_msg("TX: ", msg)
   
    bus.send(msg)
 
bus = can.interface.Bus(bustype='socketcan', channel='can1')
 
VCUnodeID = 0x49

sdo_block_count = 127
sdo_block_download = False 
sdo_block_index = 1
sdo_index_lsb = 0
sdo_index_msb = 0
sdo_subindex = 0
sdo_block_calc_crc = 0

sdo_block_download_data = []

try:
    while(True):
        msg_rx = bus.recv()

        if(msg_rx is None):
            print("No message received")
            continue

        if(msg_rx.arbitration_id != (0x600 + VCUnodeID)):
            continue

        if(msg_rx.dlc != 8):
            print("Invalid dlc size")
            continue

        if sdo_block_download:  # sdo message has a index instead of a header and data bytes, process differently
            index = msg_rx.data[0] & 0x7F
            segment_end = (msg_rx.data[0] & 0x80) == 0x80

            if(index != sdo_block_index):
                # send abort message
                msg_tx = can.Message(arbitration_id=0x580 + VCUnodeID,
                        data = [0x80, sdo_index_lsb, sdo_index_msb, sdo_subindex, 0xFF, 0, 0, 0xFF],
                        is_extended_id=False)

                sdo_block_download = False

                print("RX: Invalid SDO Block Index")
                sdo_send(msg_tx)
            else:

                s = ""
                for i in range(1, 8):
                    s += "{:02x} ".format(msg_rx.data[i])
                    sdo_block_download_data.append(msg_rx.data[i])

                print("RX: " + "{:03d}: ".format(index) + s)

                sdo_block_index += 1
       
                if(segment_end):
                    sdo_block_download = False
                    print("RX: SDO Block Download End Requested")

                    msg_tx = can.Message(arbitration_id= 0x580 + VCUnodeID,
                            data = [0xA2, index, sdo_block_count, 0, 0, 0, 0, 0],
                            is_extended_id=False)
                    sdo_send(msg_tx)                  


                # check end of block segment
                elif(index == sdo_block_count):   
                    sdo_block_index = 1
                    
                    msg_tx = can.Message(arbitration_id= 0x580 + VCUnodeID,
                            data = [0xA2, index, sdo_block_count, 0, 0, 0, 0, 0],
                            is_extended_id=False)
                    sdo_send(msg_tx)                  

        else:

            print_msg("RX: ", msg_rx)

            if(msg_rx.data[0] == 0xC6):    # SDO Block Download initiate Request

                sdo_index_lsb =  msg_rx.data[1]
                sdo_index_msb =  msg_rx.data[2]
                sdo_subindex =  msg_rx.data[3] 
                sdo_size = msg_rx.data[7] << 24 | msg_rx.data[6] << 16 | msg_rx.data[5] << 8 | msg_rx.data[4]

                msg_tx = can.Message(arbitration_id=0x580 + VCUnodeID,
                        data = [0xA4, sdo_index_lsb, sdo_index_msb, sdo_subindex, sdo_block_count, 0, 0, 0],
                        is_extended_id=False)

                sdo_block_index = 1
                sdo_block_calc_crc = 0
                sdo_block_download = True
                sdo_send(msg_tx)
                sdo_block_download_data = []

                print("RX: SDO Block Download Size: ", sdo_size)

            elif((msg_rx.data[0] & 0xC1)== 0xC1):    # SDO Block Download End Request
                last_data_bytes_inv = (msg_rx.data[0] & 0x1C) >> 2
                sdo_block_expected_crc = msg_rx.data[2] << 8 | msg_rx.data[1]

                # remove non valid elements
                sdo_block_download_data = sdo_block_download_data[:-last_data_bytes_inv]    # remove last elements indicated by this message
                s = ""
                new_size = len(sdo_block_download_data)
                for i in range(0, new_size):
                    s += "{:02X} ".format(sdo_block_download_data[i])

              #  print("Data: \r\n", s)
                # calculate crc

                sdo_block_calc_crc = crc16_ccitt(sdo_block_download_data, 0)

                print("CRC Expected: {:04X}, CRC Calculated: {:04X}".format(sdo_block_expected_crc, sdo_block_calc_crc))

                if(sdo_block_calc_crc == sdo_block_expected_crc):
                    msg_tx = can.Message(arbitration_id=0x580 + VCUnodeID,
                            data = [0xA1, 0, 0, 0, 0, 0, 0, 0],
                            is_extended_id=False)
                else:
                    msg_tx = can.Message(arbitration_id=0x580 + VCUnodeID,
                            data = [0x80, 0, 0, 0, 0, 0, 0xFF, 0],      # should be invalid CRC abort message
                            is_extended_id=False)
                sdo_send(msg_tx)
            
            elif(msg_rx.data[0] == 0x40):    # SDO Expedite Upload Request

                sdo_index_lsb =  msg_rx.data[1]
                sdo_index_msb =  msg_rx.data[2]
                sdo_subindex =  msg_rx.data[3] 
                sdo_size = 4

                sdo_index = sdo_index_msb << 8 | sdo_index_lsb

                try:
                    
                    sdo_value = object_dictionary[sdo_index]

                    print("RX: Request OD Index: {:04X} SubIndex: {:02X} Size: {} Value: {:08X}".format(sdo_index, sdo_subindex, sdo_size, sdo_value))

                    data_bytes = struct.pack("<I", sdo_value)
                    msg_tx = can.Message(arbitration_id=0x580 + VCUnodeID,
                            data = [0x43, sdo_index_lsb, sdo_index_msb, sdo_subindex, data_bytes[0], data_bytes[1],data_bytes[2],data_bytes[3]],
                            is_extended_id=False)
                except:
 
                    msg_tx = can.Message(arbitration_id=0x580 + VCUnodeID,
                            data = [0x80, sdo_index_lsb, sdo_index_msb, sdo_subindex, 0xFF, 0x00, 0x00, 0xFF],
                            is_extended_id=False)      

                sdo_send(msg_tx)

            elif((msg_rx.data[0] & 0x23)== 0x23):    # SDO Expedite Download Request
                last_data_bytes_inv = (msg_rx.data[0] & 0x1C) >> 2
                sdo_index = msg_rx.data[2] << 8 | msg_rx.data[1]
                sdo_subindex = msg_rx.data[3]
 
                sdo_value = (msg_rx.data[7] << 24 | msg_rx.data[6] << 16 | msg_rx.data[5] << 8 | msg_rx.data[4]) & (0xFFFFFFFF >> (last_data_bytes_inv * 8))

                object_dictionary[sdo_index] = sdo_value

                print("RX: Request OD Index: {:04X} SubIndex: {:02X} Size: {} Value: {:08X}".format(sdo_index, sdo_subindex, sdo_size, sdo_value))
 

                msg_tx = can.Message(arbitration_id=0x580 + VCUnodeID,
                        data = [0x60, msg_rx.data[1], msg_rx.data[2], msg_rx.data[3], 0, 0, 0, 0],
                        is_extended_id=False)

                sdo_send(msg_tx)

    
except can.CanError:
    print("Message NOT received")



