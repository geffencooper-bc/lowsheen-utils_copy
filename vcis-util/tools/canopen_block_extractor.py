#!/usr/bin/env python
# ============================================================================
# Copyright 2020 Brain Corporation. All rights reserved. Brain
# Corporation proprietary and confidential.
# ALL ACCESS AND USAGE OF THIS SOURCE CODE IS STRICTLY PROHIBITED
# WITHOUT EXPRESS WRITTEN APPROVAL FROM BRAIN CORPORATION.
# Portions of this Source Code and its related modules/libraries
# may be governed by one or more third party licenses, additional
# information of which can be found at:
# https://info.braincorp.com/open-source-attributions
# ============================================================================

from __future__ import print_function
from __future__ import absolute_import
from __future__ import division
import argparse
import os
from enum import Enum
from datetime import datetime


class CANFrame(object):
    def __init__(self):
        self.index = 0
        self.system_timestamp = ""
        self.timestamp = 0
        self.identifier = 0x00
        self.data = []
        self.dlc = 0
        self.rtr = 0

class USBCANToolDecoderIterator(object):
    
    def __init__(self, csv_file):
        self.start = True
        self.csv_file = csv_file
        self.start = True
        self.skip = True

    def __iter__(self):
        return self
        
    def next(self):

        if(self.start):
            self.file = open(self.csv_file, 'r')
            self.start = False

        if(self.file is None):
            raise StopIteration
            
        if(self.skip):
            # skip first line
            line = self.file.readline()            
            if(line is None or line == ""):
                self.file.close()
                raise StopIteration
            self.skip = False

        line = self.file.readline()

        if(line is not None and line != ""):
            line = line.strip()
 
            frame = CANFrame()

            items = [x.strip() for x in line.split(',')]

            frame.index = int(items[0], 10)
            frame.system_timestamp = (items[1][1:]).strip("\"")
            frame.timestamp = int(items[2], 16)

            frame.identifier = int(items[5], 16)
            frame.dlc = int(items[8], 16)
            frame.rtr = 1 if items[7] == "Remote" else 0

            data = items[9].split(' ')[1:]
            frame.data = [int(x, 16) for x in data]

    
          #  print(frame.index)
        
            return frame

        else:
            self.file.close()
            raise StopIteration

class USBCANToolDecoder(object):

    def decode(self, input_filename):
        return USBCANToolDecoderIterator(input_filename)


class SDOBlockDownloadStates(Enum):
    INITIATE = 0,
    INITIATE_REPONSE = 1,
    DOWNLOAD = 2,
    DOWNLOAD_RESPONSE = 3,
    END = 4,
    END_RESPONSE = 5,

class CANopenBlockBinaryExtractor(object):

    def __init__(self, decoder):
        self.decoder = decoder


        self.sdo_msgs = {}

        self.sdo_msgs[0xC6] = "SDO Block Download Initiate Request (with CRC, Size Indicator)"
        self.sdo_msgs[0xC4] = "SDO Block Download Initiate Request (with CRC)"
        self.sdo_msgs[0xC2] = "SDO Block Download Initiate Request (with Size Indicator)"
        self.sdo_msgs[0xA4] = "SDO Block Download Initiate Response (with CRC)"
        self.sdo_msgs[0xA0] = "SDO Block Download Initiate Response (without CRC)"        
        self.sdo_msgs[0xA2] = "Block Download End Response"
        self.sdo_msgs[0xC1] = "SDO Block Download End Request (8 bytes)"
        self.sdo_msgs[0xC5] = "SDO Block Download End Request (7 bytes)"
        self.sdo_msgs[0xC9] = "SDO Block Download End Request (6 bytes)"
        self.sdo_msgs[0xCD] = "SDO Block Download End Request (5 bytes)"
        self.sdo_msgs[0xD1] = "SDO Block Download End Request (4 bytes)"
        self.sdo_msgs[0xD5] = "SDO Block Download End Request (3 bytes)"
        self.sdo_msgs[0xD9] = "SDO Block Download End Request (2 bytes)"
        self.sdo_msgs[0xDD] = "SDO Block Download End Request (1 bytes)"
        self.sdo_msgs[0xA1] = "SDO Block Download End Response"
        self.sdo_msgs[0x80] = "SDO Abort"
        self.sdo_msgs[0x40] = "SDO Expedite Upload Request"
        self.sdo_msgs[0x43] = "SDO Expedite Upload Response (4 bytes)"
        self.sdo_msgs[0x45] = "SDO Expedite Upload Response (3 bytes)"
        self.sdo_msgs[0x47] = "SDO Expedite Upload Response (2 bytes)"
        self.sdo_msgs[0x4F] = "SDO Expedite Upload Response (1 bytes)"
        self.sdo_msgs[0x23] = "SDO Expedite Download Request (4 bytes)"
        self.sdo_msgs[0x27] = "SDO Expedite Download Request (3 bytes)"
        self.sdo_msgs[0x2B] = "SDO Expedite Download Request (2 bytes)"
        self.sdo_msgs[0x2F] = "SDO Expedite Download Request (1 bytes)"
        self.sdo_msgs[0x60] = "SDO Expedite Download Response"




    def decode_sdo_frame(self, frame, state):

        msg_data = ""
        for i in range(0, frame.dlc):
            if i > 0:
                msg_data += ", "
            msg_data += "{:02X}".format(frame.data[i])

        if state == SDOBlockDownloadStates.DOWNLOAD:
            msg_name = "SDO Block Download"
        else:
            msg_name = self.sdo_msgs.get(frame.data[0], "Unknown: " + hex(frame.data[0]))

        return  msg_name + ": " + msg_data


    def extract_file(self, input_filename, node=0x49):
   
        client_sdo_index = 0
        client_sdo_sub_index = 0
        state = SDOBlockDownloadStates.INITIATE
        sequence = 0    # wait for start of block
        sequence_max = 0
        client_file_size = 0
        binary_data = []
        end_transfer = False
        output_filename = None
        flow_sequence = []
        blocks = 0
        verify_crc = False

        for frame in self.decoder.decode(input_filename):

            # client

            if(frame.identifier == 0x600 + node):
                flow_sequence.append("Client: " + self.decode_sdo_frame(frame, state))
                if(state == SDOBlockDownloadStates.INITIATE):      # Block Download Initiate
                    if((frame.data[0] & 0xC2) == 0xC2):
                        client_sdo_index = frame.data[2] << 8 | frame.data[1]
                        client_sdo_sub_index = frame.data[3]
                        client_file_size = frame.data[7] << 24 | frame.data[6] << 16 | frame.data[5] << 8 | frame.data[4]
                        object_str = "{:04X}-{:02X}".format(client_sdo_index, client_sdo_sub_index)
                        file_without_ext = os.path.splitext(input_filename)[0]
                        output_filename = file_without_ext + "_" + object_str
                        state = SDOBlockDownloadStates.INITIATE_REPONSE 
                        binary_data = []                        
                        print("Found Block Download of {} size: {} KB ({} bytes)".format(object_str, int(client_file_size / 1024), client_file_size))

                elif(state == SDOBlockDownloadStates.DOWNLOAD ):    # Block Download
                    
                    end_transfer = ((frame.data[0] & 0x80) == 0x80)
                    client_sequence = frame.data[0] & 0x7F

                    sequence += 1

                    if(client_sequence != sequence):
                        print("DOWNLOAD: Block Transfer Sequence not followed. Client. Expected: {} Received: {} Index: {} Max: {}".format(sequence, client_sequence, frame.index, sequence_max))
                        return
                 
                    for i in range(1, frame.dlc):
                        binary_data.append(frame.data[i])

                    if(sequence >= sequence_max or end_transfer):
                        state = SDOBlockDownloadStates.DOWNLOAD_RESPONSE

                    if(sequence == 1):
                       # print("DOWNLOAD: Begin Block {}".format(blocks))
                        blocks += 1
 
                elif(state == SDOBlockDownloadStates.END): # Block Download End
                    if((frame.data[0] & 0xC1) == 0xC1):
                        last_bytes_no_data = (frame.data[0] & 0x1C) >> 2  # 'CCSnnnXX'
                        binary_data = binary_data[:-last_bytes_no_data]
                        state = SDOBlockDownloadStates.END_RESPONSE
                      #  print("END: Block End Requested. Ignore Last: {} byte(s)".format(last_bytes_no_data))


            #server

            elif(frame.identifier == 0x580 + node):
                flow_sequence.append("Server: " + self.decode_sdo_frame(frame, state))
                if(frame.data[0] == 0x80):  # abort from server                    
                    print("Unexpected SDO Abort Found")
                    return                
                if(state == SDOBlockDownloadStates.INITIATE_REPONSE):
           
                    if((frame.data[0] & 0xA0) == 0xA0):    # SDO Block Download Initiate Response
                        server_sdo_index = frame.data[2] << 8 | frame.data[1]
                        server_sdo_sub_index = frame.data[3]                    
                        sequence_max = frame.data[4]

                        if(server_sdo_index != client_sdo_index or server_sdo_sub_index != client_sdo_sub_index):
                            print("Index/SubIndex Mismatch SDO Block Download Initiate Response")
                            return
                        elif(sequence_max > 0x7F):
                            print("Sequence value too big, invalid")
                            return
                      #  print("Server Accepted Block Download")
                        state = SDOBlockDownloadStates.DOWNLOAD
                        sequence = 0

                elif(state == SDOBlockDownloadStates.DOWNLOAD_RESPONSE):
                    if((frame.data[0]) == 0xA2): # Block Download End Response
                        server_sequence = frame.data[1]
                        sequence_max = frame.data[2]

                        if(server_sequence != sequence):
                            print("DOWNLOAD_RESPONSE: Block Transfer Sequence not followed. Server. Expected: {} Received: {}".format(sequence, server_sequence))
                            return

                        if(end_transfer):
                            state = SDOBlockDownloadStates.END      
                        else:
                            state = SDOBlockDownloadStates.DOWNLOAD
                            sequence = 0
                elif(state == SDOBlockDownloadStates.END_RESPONSE):
                    if((frame.data[0]) == 0xA1):

                        final_filename = output_filename # + "_" + datetime.now().strftime("%m%d%Y-%H%M%S")

                        # print("Writing Text Output File: {}".format(final_filename + ".txt"))    
                        # with open(final_filename + ".txt", "w") as f:
                        #     for i in range(0, len(binary_data)):
                        #         f.write("{:02X} ".format(binary_data[i]))

                        #         # segments come in 7 byte chunks, keep text file data aligned with input file
                        #         if((i + 1) % 7 == 0 and i != 0):
                        #             f.write('\n')

                        print("Writing Decoded Output File: {}".format(final_filename + "_decoded.txt"))    
                        with open(final_filename + "_decoded.txt", "w") as f:
                            f.write("SDO Block Decoder. Node: {:02X}\r\n".format(node))

                            for s in flow_sequence:
                                f.write(s + "\r\n")   

                        print("Writing Binary Output File: {}".format(final_filename + ".bin"))  
                        with open(final_filename + ".bin", "wb") as f:
                            f.write(bytearray(binary_data))                             
                        state = SDOBlockDownloadStates.INITIATE
                        
                        flow_sequence = []                     


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='CANopen Block Download Extractor')
    parser.add_argument('file', help='Input file to parse')
   # parser.add_argument('--format', default='USB-CAN-TOOL', help="File format. Available: USB-CAN-TOOL") 
    
    args = parser.parse_args()

    decoder = USBCANToolDecoder()

    extractor = CANopenBlockBinaryExtractor(decoder)

    extractor.extract_file(args.file, node=0x49)