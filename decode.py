#!/usr/bin/env python3
from sampler.scp_file import ScpFile, ScpFlag, ScpHeader, ScpHeads, ScpRevolution, ScpTrack

from sampler.flux_reader import FluxReader
import sys

flux = FluxReader("/dev/ttyACM0")

# Construct an SCP file from the decoded disc
with open('track.scp', 'wb') as scp_file:
    scp_file.write(flux.read_side(80).pack())