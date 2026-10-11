#!/usr/bin/env python3
"""Source-only U5 release-preparation gate. No audio/microphone or deployment actions."""
from pathlib import Path
import subprocess

U4 = "56399f8c47030eb8540fffd0dbf71b5f62635872"
SOURCE_FILES = (
    "index.html", "u2.css", "u2.js", "u3.css", "u3.js", "u4.css", "u4.js",
    "tests/u1_smoke.py", "tests/u2_smoke.py", "tests/u3_smoke.py", "tests/u4_smoke.py",
    ".github/workflows/u2-ui.yml", ".github/workflows/u3-ui.yml",
    ".github/workflows/u4-ui.yml", "README.md",
)
ALLOWED = {
    "docs/U5_HARDWARE_ACCEPTANCE.md", "docs/U5_RELEASE_AND_ROLLBACK.md",
    "tests/u5_source_gate.py", ".github/workflows/u5-source-only.yml",
}

def git(*args):
    return subprocess.run(["git", *args], check=True, capture_output=True, text=True).stdout.strip()

def main():
    assert git("rev-parse", "--is-inside-work-tree") == "true"
    assert subprocess.run(["git", "merge-base", "--is-ancestor", U4, "HEAD"]).returncode == 0, "U4 ancestor missing"
    changed = set(filter(None, git("diff", "--name-only", U4, "HEAD").splitlines()))
    assert changed, "U5 should be a reviewable source-only change"
    assert changed <= ALLOWED, f"Unexpected changed files outside U5 prep: {sorted(changed - ALLOWED)}"
    for file in SOURCE_FILES:
        assert git("rev-parse", f"{U4}:{file}") == git("rev-parse", f"HEAD:{file}"), f"Protected file changed: {file}"
    html = Path("index.html").read_text(encoding="utf-8")
    for asset in ("u2", "u3", "u4"):
        assert html.count(f'href="{asset}.css"') == 1
        assert html.count(f'src="{asset}.js"') == 1
    hardware = Path("docs/U5_HARDWARE_ACCEPTANCE.md").read_text(encoding="utf-8")
    release = Path("docs/U5_RELEASE_AND_ROLLBACK.md").read_text(encoding="utf-8")
    for item in ("H1-A","H1-B","H1-C","H1-D","H1-E","NOT TESTED","OWNER"):
        assert item.lower() in hardware.lower(), f"Missing hardware gate {item}"
    for item in ("karaoke.thiepn.dev", U4, "9b1d39f6110baf581f4598d45b2fba1258db18c0",
                 "8fef111f-5aab-4bdd-941c-72abecc0ebef", "NOT GIVEN", "SINGLE"):
        assert item in release, f"Missing release prerequisite: {item}"
    print("PASS U5 source-only guard: U4 ancestor intact, all protected application/tests files identical")
    print("PASS acceptance and rollback documentation; H1 remains NOT TESTED, release not approved")

if __name__ == "__main__":
    main()
