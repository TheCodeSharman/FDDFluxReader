from serial import Serial
from io import TextIOWrapper, BufferedRWPair
from sampler.scp_file import ScpFile, ScpFlag, ScpHeader, ScpHeads, ScpRevolution, ScpTrack
from sampler.sample_decoder import SampleDecoder

class FluxReader:
    debug = True
    START_SAMPLING = b'====== sampling started'
    END_SAMPLING = b'====== sampling stopped'

    def _readline(self):
        line = self._device.readline()
        if self.debug: print("READ: ", line)
        return line
    
    def _writeline(self,line):
        self._device.write(line)
        if self.debug: print("WRITE: ", line)

    def _exec(self,command):
        self._writeline(command+b"\n")
        self._expect(command+b"\n")  # echo is turned on so we expect the command exhoed back

    def _expect(self, expected):
        line = self._readline()
        if not line.startswith(expected):
            raise Exception(f"Expected bytes starting with {expected} but got {line}")

    def _wait_for(self, text):
        line = b''
        while not line.startswith(text):
            line = self._readline()

    def _wait_for_prompt(self):
        self._wait_for(b"> ")

    def __init__(self, serialDevice, ):
        self._device = Serial(serialDevice, 115200, timeout=1)
        print("Connecting to flux reader device...")
        self._wait_for(b"FDDFluxReader")
        self._wait_for_prompt()

    def home(self):
        self._exec(b"home")
        self._expect(b"Success!")
        self._wait_for_prompt()

    def seek_track(self,track):
        self._exec(b"seek_track " + bytes(str(track), 'utf-8'))
        self._expect(b"Success!")
        self._wait_for_prompt()

    def read_track(self):
        decoder = SampleDecoder()
        self._exec(b"start_sampling")
        self._wait_for(self.START_SAMPLING)
        line = self._readline()
        while not line.startswith(self.END_SAMPLING):
            decoder.processLine(line)
            line = self._readline()
        return ScpTrack(
                track_number = 0,
                revolutions = [ ScpRevolution( 
                        index_time_ns = 1667*100*1000,
                        bitcell_data = decoder.samples
                    )
                ]
            )
    
    def read_side(self, total_tracks):
        tracks = []
        
        print("Homing...")
        self.home()
        
        print("reading track", end="")
        for track in range(0,total_tracks-1):
            print(f" {track}")
            self.seek_track(track)
            tracks.append(self.read_track())
        
        print("\nSuccess!")
        return ScpFile(
            header = ScpHeader( 
                start_track = 0,
                end_track = 0,
                heads = ScpHeads.BOTTOM,
                flags = ScpFlag.INDEX | ScpFlag.RPM_360 | ScpFlag.TPI_96
                ), 
            tracks = tracks)
        
            

        


