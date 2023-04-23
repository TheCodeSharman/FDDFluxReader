#!/usr/bin/env python3
import base64

start_sampling = '====== sampling started ======'
end_sampling = '====== sampling stopped ======'

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
            print(sample*25/1000, 'ms')

decoder = PinSampleDecoder()
with open('platformio-device-monitor-230423-160853.log', 'r') as file:
    line = ""
    while not line.startswith(start_sampling):
        line = file.readline()
    
    line = file.readline()
    while not line.startswith(end_sampling):
        decoder.processbuffer(base64.standard_b64decode(line))
        line = file.readline()
        
decoder.dump()






