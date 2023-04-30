#!/usr/bin/env python3
from sampler.scp_file import ScpFile, ScpFlag, ScpHeader, ScpHeads, ScpRevolution, ScpTrack
from sampler.sample_decoder import SampleDecoder
import sys

start_sampling = '====== sampling started'
end_sampling = '====== sampling stopped'

if ( len(sys.argv) != 2):
    print("Syntax: decode.py <filename>")
    exit()

ascii_file = sys.argv[1]

decoder = SampleDecoder()
with open(ascii_file, 'r') as file:
    line = ""
    while not line.startswith(start_sampling):
        line = file.readline()
    
    line = file.readline()
    while not line.startswith(end_sampling):
        decoder.processLine(line)
        line = file.readline()
        
# Construct an SCP file from the decoded track
with open('track.scp', 'wb') as scp_file:
    scp_file.write(ScpFile(
        header = ScpHeader( 
            start_track = 0,
            end_track = 0,
            heads = ScpHeads.BOTTOM,
            flags = ScpFlag.INDEX | ScpFlag.RPM_360 | ScpFlag.TPI_96
            ), 
        tracks = [
            ScpTrack(
                track_number = 0,
                revolutions = [ ScpRevolution( 
                        index_time_ns = 1667*100*1000,
                        bitcell_data = decoder.samples
                    )
                ]
            )
        ]).pack())