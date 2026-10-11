#!/usr/bin/env python3
"""ROOMTONE U8 human-evidence *shape* validator, NOT a truth or release verifier.

Never changes a gate, merges, deploys, calls APIs, or processes screenshots.
Human observations must be reported by the actual witness outside this source
template. Synthetic unit tests cannot satisfy HU7-01 through HU7-08.
"""
from __future__ import annotations

from datetime import datetime
from pathlib import Path
import json
import re
import sys

QUALIFIED_U7 = "1c7dd8f10d2f2fcccf1ac9809f1f9047dbb4666a"
EXPECTED = [f"HU7-{n:02d}" for n in range(1, 9)]
# Fixed requirements; owner observations belong in 'observation', not revised scenarios.
SCENARIOS = {
    "HU7-01": "Android Chrome portrait Stage: in-flow Queue/Mixer, sticky navigation Mute, no Play/Next occlusion, focus and scroll",
    "HU7-02": "Android Chrome Queue/Mixer open/close and keyboard or tap focus recovery",
    "HU7-03": "Android Chrome Songs search, filters, and synthetic local LRC without microphone",
    "HU7-04": "Samsung Internet Stage, Queue/Mixer and emergency Mute visibility",
    "HU7-05": "Desktop Chrome Stage, library, fullscreen lyrics, contrast and queue",
    "HU7-06": "Physical keyboard: Tab, Shift+Tab, Enter, Space, Escape and focus return",
    "HU7-07": "Actual assistive technology: TalkBack/NVDA announcements, focus and filter semantics",
    "HU7-08": "Explicit owner visual/release decision, separate from automated tests",
}
KEYS = {
    "id", "scenario", "status", "observer", "observed_at", "device",
    "browser", "observation", "attests_direct_observation",
    "private_evidence_id", "evidence_sha256",
}
ROOT_KEYS = {
    "schema_version", "project", "phase", "observed_source_commit",
    "release_gate", "evidence_kind", "tests",
}
HEX64 = re.compile(r"^[0-9a-f]{64}$")
# Opaque owner-managed local ID only. No scheme:// URLs, filesystem paths or network hosts.
PRIVATE_REF = re.compile(r"^owner-local:[A-Za-z0-9][A-Za-z0-9_.-]{2,80}$")


def validate(payload: object) -> dict:
    """Raise ValueError on malformed/unattributed claims; never establish truth."""
    if not isinstance(payload, dict) or set(payload) != ROOT_KEYS:
        raise ValueError("Unexpected or missing evidence document fields")
    if payload["schema_version"] != 1 or payload["project"] != "ROOMTONE" or payload["phase"] != "U8":
        raise ValueError("Incorrect intake document schema or project")
    if payload["observed_source_commit"] != QUALIFIED_U7:
        raise ValueError("Evidence is not pinned to exact qualified U7 commit")
    if payload["release_gate"] != "NO_GO":
        raise ValueError("Evidence intake cannot authorize a release")
    if payload["evidence_kind"] != "HUMAN_OWNER_INTAKE_TEMPLATE_NOT_AN_ACCEPTANCE":
        raise ValueError("Intake label must not imply attested acceptance")
    cases = payload["tests"]
    if not isinstance(cases, list) or len(cases) != len(EXPECTED):
        raise ValueError("Exactly eight HU7 witness gates must be present")
    if [case.get("id") for case in cases if isinstance(case, dict)] != EXPECTED:
        raise ValueError("HU7 ids must be unique, complete and ordered")

    completed = 0
    results = {}
    for case in cases:
        if set(case) != KEYS:
            raise ValueError(f"{case.get('id')}: unexpected/missing fields")
        test_id = case["id"]
        scenario = case["scenario"]
        if not isinstance(scenario, str) or scenario != SCENARIOS[test_id]:
            raise ValueError(f"{test_id}: scenario differs from the pinned human test requirement")
        status = case["status"]
        allowed = {"NOT_GIVEN", "APPROVED", "REJECTED"} if test_id == "HU7-08" else {"NOT_TESTED", "PASS", "FAIL"}
        if not isinstance(status, str) or status not in allowed:
            raise ValueError(f"{test_id}: unsupported status {status!r}")
        missing = status in ("NOT_TESTED", "NOT_GIVEN")
        fields = ("observer", "observed_at", "device", "browser", "observation",
                  "private_evidence_id", "evidence_sha256")
        if missing:
            if any(case[name] is not None for name in fields):
                raise ValueError(f"{test_id}: untested case must not contain fake witness fields")
            if case["attests_direct_observation"] is not False:
                raise ValueError(f"{test_id}: untested case must have false attestation")
        else:
            completed += 1
            if case["attests_direct_observation"] is not True:
                raise ValueError(f"{test_id}: direct witness attestation is required")
            for field in ("observer", "device", "browser"):
                if not isinstance(case[field], str) or len(case[field].strip()) < 2:
                    raise ValueError(f"{test_id}: missing actual witness {field}")
            if not isinstance(case["observation"], str) or len(case["observation"].strip()) < 20:
                raise ValueError(f"{test_id}: actual observed outcome is required")
            try:
                when = datetime.fromisoformat(case["observed_at"].replace("Z", "+00:00"))
                if when.tzinfo is None or when.utcoffset() is None:
                    raise ValueError("missing offset")
            except (ValueError, TypeError, AttributeError) as exc:
                raise ValueError(f"{test_id}: observed_at requires ISO-8601 timezone") from exc
            ref = case["private_evidence_id"]
            digest = case["evidence_sha256"]
            if (ref is None) != (digest is None):
                raise ValueError(f"{test_id}: evidence pointer and SHA256 must both be set or both null")
            if ref is not None:
                if not isinstance(ref, str) or not PRIVATE_REF.fullmatch(ref):
                    raise ValueError(f"{test_id}: use a private local evidence identifier, not a raw URL")
                if not isinstance(digest, str) or not HEX64.fullmatch(digest):
                    raise ValueError(f"{test_id}: evidence SHA256 must be exact lowercase hex")
        results[test_id] = status
    # A machine could fill a structurally valid but fictitious witness record:
    # this validator DOES NOT pronounce any claimed PASS genuine or approved.
    return {"document_shape": "VALID", "claimed_observed": completed,
            "statuses": results, "release": "NO_GO", "human_truth_verified": False}


def main(argv: list[str]) -> int:
    if len(argv) != 2:
        print("Usage: python scripts/u8_validate_evidence.py <local-json>", file=sys.stderr)
        return 2
    try:
        doc = json.loads(Path(argv[1]).read_text(encoding="utf-8"))
        result = validate(doc)
    except (OSError, json.JSONDecodeError, ValueError) as exc:
        print(f"REJECTED_INTAKE: {exc}", file=sys.stderr)
        return 1
    print(json.dumps(result, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
