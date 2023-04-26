from base64 import standard_b64decode

class SampleDecoder: 
    def __init__(self):
        self.samples = []

    def processLine(self, line):
        buffer = standard_b64decode(line)
        ptr = 0
        while ptr < len(buffer):
            ptr = self.decodeSample(buffer, ptr)

    def decodeSample(self, buffer, ptr):
        sample = 100
        byte = 0x80 
        shift = 0
        while byte & 0x80: 
            byte = buffer[ptr]
            sample += (byte & 0x7f) << shift
            shift += 7
            ptr += 1
        self.samples.append(sample)
        return ptr

    def dump(self):
        for sample in self.samples:
            print(sample*25/1000, '\xB5s')