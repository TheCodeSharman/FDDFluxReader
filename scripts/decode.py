#!/usr/bin/env python3
from base64 import standard_b64decode
from enum import Flag
from binascii import crc32
from dataclasses import dataclass

start_sampling = '====== sampling started ======'
end_sampling = '====== sampling stopped ======'



class ScpFlag(Flag):
    INDEX      = 0x1  # is set if the data is syncronised to index hole
    TPI_96     = 0x2  # TPI flag; set is 96tpi, clear is 48tpi
    RPM_360    = 0x4  # RPM flag; set is 360 rpm
    NORMALIZED = 0x8  # TYPE flag; set is normalized
    WRITE      = 0x10 # MODE flag; set is read/write
    FOOTER     = 0x20 # is set there is footer

@dataclass
class ScpHeader:
    def __init__(self, 
            disk_type = 0x8000,    # disk type ( man_Other, disk_360 )
            revolutions = 1, 
            start_track = 1, 
            end_track = 80, 
            flags = 0,
            bitcell_width = 0,
            heads = 0,             # 0 = both, 1 = side 0 (bottom), 2 = side 1 (top)
            resolution = 0        # 0 = 25ns, otherwise 25ns + n*25ns
        ):
        self.version = 0x19
        self.disktype = disk_type
        self.revolutions = revolutions
        self.starttrack = start_track
        self.endtrack = end_track
        self.flags = flags,
        self.bitcell_width = bitcell_width
        self.heads = heads
        self.resolution = resolution
    
    def write(self, buffer):
        buffer.write("SCP")
        buffer.write(bytes(
            [
                self.version, 
                self.disk_type, 
                self.revolutions, 
                self.start_track, 
                self.end_track, 
                self.flags,
                self.bitcell_width,
                self.heads,
                self.resolution
            ]))

class ScpRevolution:
    def __init__(self, index_time_ns, bitcell_data):
        self.index_time = index_time_ns/25
        self.bitcell_length = len(bitcell_data)
        self.bitcell_data = bitcell_data

class ScpTrackHeader:
    def __init__(self, number):
        self.number = number
        self.revolutions = []
        pass

    def add_revolution(self, revolution):
        self.revolutions.append(revolution)

    def write(self,buffer):
        buffer.write('TRK')
        buffer.write(bytes([self.number]))
        buffer.write


class ScpFile:
    def __init__(self, header = ScpHeader()):
        self.header = header
        self.tracks = []

    def add_track(self, track):
        self.tracks.append(track)

    def write(self,buffer):
        self.header.write(buffer)
        # write 4 bytes of checksum
        # write track header
        # write track data


class PinSampleDecoder: 
    def __init__(self):
        self.samples = []

    def processbuffer(self, buffer):
        ptr = 0
        while ptr < len(buffer):
            ptr = self.decodesample(buffer, ptr)

    def decodesample(self, buffer, ptr):
        sample = 100
        byte = 0x80 
        while byte & 0x80: 
            byte = buffer[ptr]
            sample += (byte & 0x7f)
            ptr += 1
        self.samples.append(sample)
        return ptr

    def dump(self):
        for sample in self.samples:
            print(sample*25/1000, '\xB5s')

decoder = PinSampleDecoder()
with open('platformio-device-monitor-230423-160853.log', 'r') as file:
    line = ""
    while not line.startswith(start_sampling):
        line = file.readline()
    
    line = file.readline()
    while not line.startswith(end_sampling):
        decoder.processbuffer(standard_b64decode(line))
        line = file.readline()
        
decoder.dump()






