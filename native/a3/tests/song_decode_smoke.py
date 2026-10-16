"""Native Windows local decode test without microphone or speakers."""
import math
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import wave
exe=Path(sys.argv[1])
with tempfile.TemporaryDirectory() as td:
 path=Path(td)/"backing.wav"
 with wave.open(str(path),"wb") as f:
  f.setnchannels(2);f.setsampwidth(2);f.setframerate(48000)
  f.writeframes(b"".join(struct.pack("<hh",
    int(8000*math.sin(2*math.pi*i/96)),int(5000*math.sin(2*math.pi*i/120)))
    for i in range(7200)))
 ok=subprocess.run([str(exe),"--probe-song","--song",str(path)],
    capture_output=True,text=True,timeout=20)
 assert ok.returncode==0,(ok.returncode,ok.stdout,ok.stderr)
 assert "DECODED_LOCAL_FRAMES 7200 AT_48000_STEREO" in ok.stdout,ok.stdout
 missing=subprocess.run([str(exe),"--probe-song","--song",str(path)+".missing"],
    capture_output=True,text=True,timeout=20)
 assert missing.returncode!=0,"Missing song was accepted"
 invalid=subprocess.run([str(exe),"--probe-song"],
    capture_output=True,text=True,timeout=20)
 assert invalid.returncode!=0,"Song not required for probe"
print("PASS Windows native WAV decode, frame count, nonexistent and absent file rejection. Physical audio NOT TESTED.")
