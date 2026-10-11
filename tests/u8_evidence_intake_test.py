"""U8 evidence intake contract tests. ALL sample witnesses here are synthetic."""
from __future__ import annotations
import copy
import json
from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))
from u8_validate_evidence import validate  # noqa: E402

TEMPLATE = ROOT / "docs/evidence/U8_HU7_ACCEPTANCE.template.json"


class IntakeContract(unittest.TestCase):
    def setUp(self):
        self.base = json.loads(TEMPLATE.read_text(encoding="utf-8"))

    def test_baseline_exactly_eight_open_human_gates(self):
        result = validate(self.base)
        self.assertEqual(result["document_shape"], "VALID")
        self.assertEqual(result["claimed_observed"], 0)
        self.assertEqual(result["release"], "NO_GO")
        self.assertIs(result["human_truth_verified"], False)
        self.assertEqual(list(result["statuses"].values()), ["NOT_TESTED"] * 7 + ["NOT_GIVEN"])

    def test_rejects_unsourced_pass_claim(self):
        p = copy.deepcopy(self.base)
        p["tests"][0]["status"] = "PASS"
        with self.assertRaisesRegex(ValueError, "attestation"):
            validate(p)

    def test_rejects_fake_witness_on_untested_test(self):
        p = copy.deepcopy(self.base)
        p["tests"][2]["observer"] = "UNVERIFIED"
        with self.assertRaisesRegex(ValueError, "untested"):
            validate(p)

    def test_rejects_non_string_status_without_traceback(self):
        # Reproduces original U8 TypeError for unhashable list/dict statuses.
        # These are synthetic malformed inputs, never owner observations.
        for invalid in ([], {}, ["PASS"], {"status": "PASS"}, None, 4):
            with self.subTest(invalid=invalid):
                payload = copy.deepcopy(self.base)
                payload["tests"][0]["status"] = invalid
                with self.assertRaisesRegex(ValueError, "unsupported status"):
                    validate(payload)

    def test_rejects_relabeling_a_required_human_scenario(self):
        # Originally accepted a long but unrelated string, allowing a user
        # to mark an easier task under a fixed HU7 test identifier.
        payload = copy.deepcopy(self.base)
        payload["tests"][0]["scenario"] = (
            "Desktop test on arbitrary interface; skip Android safety checks"
        )
        with self.assertRaisesRegex(ValueError, "pinned human test"):
            validate(payload)
        payload = copy.deepcopy(self.base)
        payload["tests"][7]["scenario"] = (
            "General opinion rather than explicit owner release decision"
        )
        with self.assertRaisesRegex(ValueError, "pinned human test"):
            validate(payload)

    def test_rejects_unauthorized_release_flag(self):
        p = copy.deepcopy(self.base)
        p["release_gate"] = "GO"
        with self.assertRaisesRegex(ValueError, "authorize"):
            validate(p)

    def test_rejects_different_app_commit(self):
        p = copy.deepcopy(self.base)
        p["observed_source_commit"] = "0" * 40
        with self.assertRaisesRegex(ValueError, "exact"):
            validate(p)

    def test_rejects_missing_and_duplicate_human_gates(self):
        p = copy.deepcopy(self.base)
        p["tests"][2]["id"] = "HU7-02"
        with self.assertRaisesRegex(ValueError, "unique"):
            validate(p)
        p = copy.deepcopy(self.base)
        p["tests"].pop()
        with self.assertRaisesRegex(ValueError, "eight"):
            validate(p)

    def test_rejects_unsanctioned_fields_and_missing_outcomes(self):
        p = copy.deepcopy(self.base)
        p["tests"][0]["screenshot_image_binary"] = "forbidden"
        with self.assertRaisesRegex(ValueError, "fields"):
            validate(p)
        p = copy.deepcopy(self.base)
        p["tests"][7]["status"] = "PASS"  # owner approval must be explicit
        with self.assertRaisesRegex(ValueError, "unsupported"):
            validate(p)

    def test_structural_acceptance_does_not_prove_a_real_human(self):
        # Deliberately SYNTHETIC unit fixture; NEVER a reported owner result.
        p = copy.deepcopy(self.base)
        x = p["tests"][0]
        x.update(status="PASS", observer="SYNTHETIC_TEST_ONLY",
                 observed_at="2026-10-11T17:30:00+02:00",
                 device="SYNTHETIC_PHONE", browser="SYNTHETIC_CHROME",
                 observation="Synthetic sample to verify input-field validation only",
                 attests_direct_observation=True,
                 private_evidence_id=None, evidence_sha256=None)
        result = validate(p)
        self.assertEqual(result["claimed_observed"], 1)
        self.assertIs(result["human_truth_verified"], False)
        self.assertEqual(result["release"], "NO_GO")

    def test_rejects_non_utc_offsetless_timestamp(self):
        p = copy.deepcopy(self.base)
        x = p["tests"][0]
        x.update(status="FAIL", observer="SYNTHETIC_TEST_ONLY",
                 observed_at="2026-10-11T17:30:00",
                 device="SYNTHETIC_PHONE", browser="SYNTHETIC_CHROME",
                 observation="Synthetic failure example for parser rejection",
                 attests_direct_observation=True)
        with self.assertRaisesRegex(ValueError, "timezone"):
            validate(p)

    def test_rejects_network_urls_and_paths_as_private_evidence_ids(self):
        # SYNTHETIC values only: no screenshot or actual owner witness.
        for ref in ("https://example.invalid/evidence.png",
                    "file:///tmp/private.png", "../private/witness.png",
                    "owner-local:https://example.invalid"):
            with self.subTest(ref=ref):
                p = copy.deepcopy(self.base)
                x = p["tests"][0]
                x.update(status="FAIL", observer="SYNTHETIC_TEST_ONLY",
                         observed_at="2026-10-11T17:30:00+02:00",
                         device="SYNTHETIC_PHONE", browser="SYNTHETIC_CHROME",
                         observation="Synthetic validation of private evidence locator only",
                         attests_direct_observation=True,
                         private_evidence_id=ref, evidence_sha256="a" * 64)
                with self.assertRaisesRegex(ValueError, "private local evidence identifier"):
                    validate(p)

    def test_rejects_private_media_pointer_without_hash(self):
        p = copy.deepcopy(self.base)
        x = p["tests"][0]
        x.update(status="FAIL", observer="SYNTHETIC_TEST_ONLY",
                 observed_at="2026-10-11T17:30:00+02:00",
                 device="SYNTHETIC_PHONE", browser="SYNTHETIC_CHROME",
                 observation="Synthetic failure example, no actual media",
                 attests_direct_observation=True,
                 private_evidence_id="owner-local:synthetic", evidence_sha256=None)
        with self.assertRaisesRegex(ValueError, "both"):
            validate(p)


if __name__ == "__main__":
    unittest.main(verbosity=2)
