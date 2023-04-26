import unittest 
from sampler.sample_decoder import SampleDecoder

class SampleDecoderTestCase(unittest.TestCase):

    def test_decode(self):
        expectedSamples = [
            199, 201, 202, 198, 135, 134, 199, 200, 3121, 137, 258, 143, 260, 138, 130, 137, 260, 141, 
            260, 140, 130, 137, 261, 141, 262, 138, 130, 137, 260, 142, 260, 140, 130, 136, 259, 142, 
            262, 137, 131, 139, 258, 141
        ]
        encodedLine = "Y2VmYiMiY2TNFyWeASugASYeJaABKaABKB4loQEpogEmHiWgASqgASgeJJ8BKqIBJR8nngEp";

        decoder = SampleDecoder()
        decoder.processLine(encodedLine)

        self.assertListEqual(decoder.samples, expectedSamples)
