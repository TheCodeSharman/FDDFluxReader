#!/usr/bin/env python3
from base64 import standard_b64decode
from enum import Flag, Enum
from binascii import crc32
from dataclasses import dataclass
from struct import pack
from typing import List

start_sampling = '====== sampling started ======'
end_sampling = '====== sampling stopped ======'

class ScpFlag(Flag):
    INDEX      = 0x1  # is set if the data is syncronised to index hole
    TPI_96     = 0x2  # TPI flag; set is 96tpi, clear is 48tpi
    RPM_360    = 0x4  # RPM flag; set is 360 rpm
    NORMALIZED = 0x8  # TYPE flag; set is normalized
    WRITE      = 0x10 # MODE flag; set is read/write
    FOOTER     = 0x20 # is set there is footer

class ScpHeads(Enum):
    BOTH   = 0
    BOTTOM = 1 # side 0
    TOP    = 2 # side 1

@dataclass
class ScpHeader:
    version: int = 0x19
    disk_type: int = 0x80
    revolutions: int = 1
    start_track: int = 1
    end_track: int = 80
    flags: ScpFlag = 0
    bitcell_width: int = 0
    heads: ScpHeads = ScpHeads.BOTH
    resolution: int = 0
    
    def pack(self):
        return pack("3s9B", b'SCP', self.version,  self.disk_type, self.revolutions, 
            self.start_track, self.end_track, self.flags.value,
            self.bitcell_width, self.heads.value, self.resolution)

@dataclass
class ScpRevolution:
    index_time_ns: int 
    bitcell_data: List[int]

    def pack_header(self, offset):
        return pack("3I", self.index_time_ns//25, len(self.bitcell_data), offset)
    
    def pack_bitcell_data(self):
        return pack('>%iH' % len(self.bitcell_data), *self.bitcell_data)

@dataclass
class ScpTrack:
    revolutions: List[ScpRevolution]
    track_number: int = 0
    
    def pack(self):
        bytes = pack('3sB', b'TRK', self.track_number)
        offset = 4 + len(self.revolutions)*12
        for revolution in self.revolutions:
            bytes += revolution.pack_header(offset)
            offset += len(revolution.bitcell_data)*2
        for revolution in self.revolutions:
            bytes += revolution.pack_bitcell_data()
        return bytes

@dataclass
class ScpFile:
    header: ScpHeader
    tracks: List[ScpTrack]

    def pack(self):
        track_offsets = bytes()
        all_track_data = bytes()
        num_tracks = self.header.end_track - self.header.start_track + 1
        
        if num_tracks != len(self.tracks):
            raise Exception("Expecting %i tracks but got %i" % (num_tracks, len(self.tracks)))
        
        offset = num_tracks*4

        for track in self.tracks:
            track_offsets += pack('<I',offset)
            track_data = track.pack()
            all_track_data += track_data
            offset += len(track_data)

        header = self.header.pack()
        tracks_and_offsets = track_offsets + all_track_data
        track_crc32 = pack('<I', crc32(tracks_and_offsets))
        return b''.join([header, track_crc32, tracks_and_offsets])

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
        
# Construct an SCP file from the decoded track
with open('track.scp', 'wb') as scp_file:
    scp_file.write(ScpFile(
        header = ScpHeader( 
            start_track = 1,
            end_track = 1,
            flags = ScpFlag.INDEX | ScpFlag.RPM_360 | ScpFlag.TPI_96
            ), 
        tracks = [
            ScpTrack(
                track_number = 1,
                revolutions = [ ScpRevolution( 
                        index_time_ns = 166*1000*1000,
                        bitcell_data = decoder.samples
                    )
                ]
            )
        ]).pack())






