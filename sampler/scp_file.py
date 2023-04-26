from dataclasses import dataclass
from struct import pack
from typing import List
from enum import Flag, Enum
from binascii import crc32

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
        return pack('<3I', self.index_time_ns//25, len(self.bitcell_data), offset)
    
    def pack_bitcell_data(self):
        return pack('>%iH' % len(self.bitcell_data), *self.bitcell_data)

@dataclass
class ScpTrack:
    revolutions: List[ScpRevolution]
    track_number: int = 0
    
    def pack(self):
        bytes = pack('<3sB', b'TRK', self.track_number)
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

    def __post_init__(self):
        self.track_table= [None] * 168
        for track in self.tracks:
            self.track_table[track.track_number] = track

    def pack(self):
        track_offsets = bytes()
        all_track_data = bytes()
        num_tracks = self.header.end_track - self.header.start_track + 1
        
        if num_tracks != len(self.tracks):
            raise Exception("Expecting %i tracks but got %i" % (num_tracks, len(self.tracks)))
        
        header = self.header.pack()

        offset = len(header) + 4 + len(self.track_table)*4

        for track in self.track_table:
            if track is None:
                track_offsets += pack('<I', 0)
            else:
                track_offsets += pack('<I',offset)
                track_data = track.pack()
                all_track_data += track_data
                offset += len(track_data)

        
        tracks_and_offsets = track_offsets + all_track_data
        track_crc32 = pack('<I', crc32(tracks_and_offsets))
        return b''.join([header, track_crc32, tracks_and_offsets])