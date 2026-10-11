#!/usr/bin/env python3
"""ROOMTONE U16 private witness-claim FORMAT validation only.

Does not validate human identity, observation truth, owner consent, release
readiness or hardware. Does not access network, modify source or write files.
User observation files must stay owner-private, never committed or uploaded.
"""
from __future__ import annotations

from datetime import datetime, timedelta, timezone
from pathlib import Path
import json
import re
import sys

from u8_validate_evidence import EXPECTED, SCENARIOS

CANDIDATE = "df3dc97134492974f210c6f6b96f20877dd441d4"
EVIDENCE = "LOCAL_OWNER_OBSERVATION_CLAIMS_UNVERIFIED"
ROOT_KEYS = {
    "schema_version", "project", "phase", "candidate_source_sha",
    "release_gate", "evidence_kind", "tests",
}
ROW_KEYS = {
    "id", "scenario", "status", "observer", "observed_at", "device",
    "browser", "observation", "attests_direct_observation",
    "private_evidence_id", "evidence_sha256",
}
LOCAL_REF = re.compile(r"^owner-local:[A-Za-z0-9][A-Za-z0-9_.-]{2,80}$")
SHA256 = re.compile(r"^[a-f0-9]{64}$")


def _require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def validate(payload: object) -> dict:
    """Validate a private HU7-01 CLAIM's shape, never certify it as observed."""
    _require(isinstance(payload, dict) and set(payload) == ROOT_KEYS,
             "Wrong root structure")
    _require(type(payload["schema_version"]) is int and payload["schema_version"] == 2,
             "Wrong schema version")
    _require(payload["project"] == "ROOMTONE" and payload["phase"] == "U16",
             "Wrong project or intake phase")
    _require(payload["candidate_source_sha"] == CANDIDATE,
             "Wrong exact qualified source SHA")
    _require(payload["release_gate"] == "NO_GO" and payload["evidence_kind"] == EVIDENCE,
             "Release gate or evidence kind was changed")
    cases = payload["tests"]
    _require(isinstance(cases, list) and len(cases) == 8,
             "Exactly eight HU7 cases required")
    _require(all(isinstance(c, dict) for c in cases) and
             [c.get("id") for c in cases] == EXPECTED,
             "HU7 IDs must be ordered, unique and complete")

    claimed = []
    states = {}
    for row in cases:
        key = row["id"]
        _require(set(row) == ROW_KEYS, f"{key}: wrong fields")
        _require(row["scenario"] == SCENARIOS[key],
                 f"{key}: immutable scenario changed")
        status = row["status"]
        allowed = {"NOT_GIVEN"} if key == "HU7-08" else (
            {"NOT_TESTED", "PASS", "FAIL"} if key == "HU7-01" else {"NOT_TESTED"})
        _require(type(status) is str and status in allowed,
                 f"{key}: unsupported status for current one-test stage")
        states[key] = status
        if status in ("NOT_TESTED", "NOT_GIVEN"):
            _require(all(row[field] is None for field in
                         ("observer", "observed_at", "device", "browser",
                          "observation", "private_evidence_id", "evidence_sha256")),
                     f"{key}: untested item contains invented witness fields")
            _require(row["attests_direct_observation"] is False,
                     f"{key}: untested item has attestation")
            continue

        claimed.append(key)
        _require(key == "HU7-01" and row["attests_direct_observation"] is True,
                 f"{key}: only directly claimed HU7-01 report permitted")
        for field, maximum in (("observer", 160), ("device", 200), ("browser", 160)):
            value = row[field]
            _require(isinstance(value, str) and 2 <= len(value.strip()) <= maximum,
                     f"{key}: invalid {field}")
        note = row["observation"]
        _require(isinstance(note, str) and 20 <= len(note.strip()) <= 3000,
                 f"{key}: missing or oversized observation")
        stamp = row["observed_at"]
        _require(isinstance(stamp, str) and len(stamp) <= 40,
                 f"{key}: invalid timestamp")
        try:
            moment = datetime.fromisoformat(stamp.replace("Z", "+00:00"))
        except ValueError as exc:
            raise ValueError(f"{key}: timestamp requires ISO8601 offset") from exc
        _require(moment.tzinfo is not None and moment.utcoffset() is not None,
                 f"{key}: timezone is required")
        _require(moment.astimezone(timezone.utc) <=
                 datetime.now(timezone.utc) + timedelta(minutes=10),
                 f"{key}: future observation cannot be claimed")
        private_id, digest = row["private_evidence_id"], row["evidence_sha256"]
        _require((private_id is None) == (digest is None),
                 f"{key}: private ID and image checksum must both exist or both null")
        if private_id is not None:
            _require(isinstance(private_id, str) and LOCAL_REF.fullmatch(private_id) is not None,
                     f"{key}: only opaque owner-local identifiers permitted, no paths or URLs")
            _require(isinstance(digest, str) and SHA256.fullmatch(digest) is not None,
                     f"{key}: invalid SHA256 digest")

    _require(len(claimed) <= 1, "Only HU7-01 may be reported at this stage")
    return {
        "format": "VALID_SHAPE_UNVERIFIED", "candidate_source_sha": CANDIDATE,
        "reported_test_ids_unverified": claimed, "claimed_statuses_unverified": states,
        "human_truth_verified": False, "owner_approval_verified": False,
        "release": "NO_GO", "next_action": "PRIVATE_OWNER_REVIEW_REQUIRED",
    }


def _no_duplicate_keys(pairs: list[tuple[str, object]]) -> dict:
    result = {}
    for key, value in pairs:
        if key in result:
            raise ValueError("Duplicate field in evidence JSON")
        result[key] = value
    return result


def _reject_nonjson_constant(token: str):
    raise ValueError("Non-standard JSON constant in witness intake")


def main(argv: list[str]) -> int:
    if len(argv) != 2:
        print("Usage: python scripts/u16_validate_private_witness.py <private-local-json>",
              file=sys.stderr)
        return 2
    try:
        document = json.loads(Path(argv[1]).read_text(encoding="utf-8"),
                              object_pairs_hook=_no_duplicate_keys,
                              parse_constant=_reject_nonjson_constant)
        safe_result = validate(document)
    except (OSError, UnicodeError, json.JSONDecodeError, ValueError) as exc:
        # Never echo the private JSON, observer, device, note, or path.
        error = str(exc)
        if isinstance(exc, OSError):
            error = "Local owner file could not be read"
        if isinstance(exc, json.JSONDecodeError):
            error = "Invalid JSON document"
        print("REJECTED_INTAKE: " + error, file=sys.stderr)
        return 1
    print(json.dumps(safe_result, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
