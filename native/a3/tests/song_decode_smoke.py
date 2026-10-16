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
 for forbidden in ("--pick-song","--setup"):
  guarded=subprocess.run([str(exe),"--probe-song","--song",str(path),forbidden],
    capture_output=True,text=True,timeout=20)
  assert guarded.returncode!=0,"Interactive mode incorrectly allowed in offline probe: "+forbidden
 invalid=subprocess.run([str(exe),"--probe-song"],
    capture_output=True,text=True,timeout=20)
 assert invalid.returncode!=0,"Song not required for probe"
# An offline session carries only audio identity, lyric times and loop state.
with tempfile.TemporaryDirectory() as td:
 session=Path(td)/"karaoke.rtk"
 session.write_text("ROOMTONE_HANDOFF_V1\n"
 "audio\tkaraoke.wav\n"
 "title\tPractice%20Set\n"
 "start-ms\t1250\n"
 "offset-ms\t-200\n"
 "loop\t1\nloop-start-ms\t1000\nloop-end-ms\t3000\n"
 "lyric\t1250\tFirst%20line\n",encoding="utf-8")
 ok=subprocess.run([str(exe),"--probe-handoff","--handoff",str(session)],
   capture_output=True,text=True,timeout=20)
 assert ok.returncode==0,(ok.returncode,ok.stdout,ok.stderr)
 assert "OFFLINE_SESSION_OK audio=karaoke.wav lyrics=1 start-ms=1250" in ok.stdout,ok.stdout
 session.write_text("../../../private",encoding="utf-8")
 bad=subprocess.run([str(exe),"--probe-handoff","--handoff",str(session)],
   capture_output=True,text=True,timeout=20)
 assert bad.returncode!=0,"Invalid handoff erroneously accepted"
print("PASS Windows native WAV decoder, offline .rtk handoff validation, rejection of invalid data. Physical audio NOT TESTED.")
