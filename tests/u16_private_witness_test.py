"""U16 strict negative security/intake tests, always synthetic data.

No real witness observations or personal records are created, fetched or sent.
"""
import copy
from datetime import datetime, timedelta, timezone
import json
from pathlib import Path
import sys
import unittest

ROOT=Path(__file__).resolve().parent.parent
sys.path.insert(0,str(ROOT/"scripts"))
from u16_validate_private_witness import validate, _no_duplicate_keys, _reject_nonjson_constant

TEMPLATE=json.loads((ROOT/"docs/evidence/U16_HU7_LOCAL_INTAKE.template.json").read_text())
U8=json.loads((ROOT/"docs/evidence/U8_HU7_ACCEPTANCE.template.json").read_text())
PIN="df3dc97134492974f210c6f6b96f20877dd441d4"


def synthetic(status="PASS"):
    """Invented structural fixture ONLY; must never be passed as human proof."""
    report=copy.deepcopy(TEMPLATE)
    c=report["tests"][0]
    c.update({
        "status":status,"observer":"Synthetic Test Fixture",
        "observed_at":datetime.now(timezone.utc).isoformat(),
        "device":"Synthetic device (unit test)",
        "browser":"Synthetic browser (unit test)",
        "observation":"Synthetic fixture only: fake UI observation, not an actual human test.",
        "attests_direct_observation":True,
    })
    return report


class U16WitnessShapeTests(unittest.TestCase):
    def reject(self,doc):
        with self.assertRaises(ValueError):
            validate(doc)

    def test_blank_template_is_unverified_and_no_go(self):
        v=validate(TEMPLATE)
        self.assertEqual(v["reported_test_ids_unverified"],[])
        self.assertFalse(v["human_truth_verified"])
        self.assertFalse(v["owner_approval_verified"])
        self.assertEqual(v["release"],"NO_GO")
        self.assertEqual([row["status"] for row in TEMPLATE["tests"]],
                         ["NOT_TESTED"]*7+["NOT_GIVEN"])

    def test_original_u8_scenarios_sha_and_data_untouched(self):
        self.assertEqual(U8["observed_source_commit"],
                         "1c7dd8f10d2f2fcccf1ac9809f1f9047dbb4666a")
        self.assertEqual(TEMPLATE["candidate_source_sha"],PIN)
        self.assertEqual([r["scenario"] for r in TEMPLATE["tests"]],
                         [r["scenario"] for r in U8["tests"]])
        self.assertEqual([r["status"] for r in U8["tests"]],
                         ["NOT_TESTED"]*7+["NOT_GIVEN"])

    def test_format_accepted_but_observation_never_certified(self):
        for status in ("PASS","FAIL"):
            v=validate(synthetic(status))
            self.assertEqual(v["reported_test_ids_unverified"],["HU7-01"])
            self.assertEqual(v["claimed_statuses_unverified"]["HU7-01"],status)
            self.assertFalse(v["human_truth_verified"])
            self.assertEqual(v["release"],"NO_GO")

    def test_reject_wrong_root_and_exact_source(self):
        for k,value in (("phase","U8"),("candidate_source_sha","1"*40),
                        ("release_gate","GO"),("evidence_kind","ATTESTED_ACCEPTANCE"),
                        ("project","Elsewhere"),("schema_version",True)):
            d=copy.deepcopy(TEMPLATE);d[k]=value;self.reject(d)
        d=copy.deepcopy(TEMPLATE);d["release_authorized"]=True;self.reject(d)

    def test_reject_scenario_substitution_and_gate_tamper(self):
        for index in range(8):
            d=copy.deepcopy(TEMPLATE)
            d["tests"][index]["scenario"]="Different easier test"
            self.reject(d)
        for status in ("PASS","FAIL","APPROVED"):
            d=copy.deepcopy(TEMPLATE);d["tests"][7]["status"]=status;self.reject(d)
        for status in ("PASS","FAIL"):
            d=copy.deepcopy(TEMPLATE);d["tests"][1]["status"]=status;self.reject(d)

    def test_reject_duplicate_gates_and_rows(self):
        for case in (
            TEMPLATE["tests"][:-1],
            [TEMPLATE["tests"][0]]*8,
            list(reversed(TEMPLATE["tests"])),
            [*TEMPLATE["tests"],TEMPLATE["tests"][7]],
        ):
            d=copy.deepcopy(TEMPLATE);d["tests"]=case;self.reject(d)
        d=copy.deepcopy(TEMPLATE);d["tests"][0]["attests_direct_observation"]=True;self.reject(d)
        d=copy.deepcopy(TEMPLATE);d["tests"][1]["observer"]="Invented";self.reject(d)

    def test_reject_unattested_fictitious_claims(self):
        d=synthetic();d["tests"][0]["attests_direct_observation"]=False;self.reject(d)
        d=synthetic();d["tests"][0]["observation"]="too short";self.reject(d)
        d=synthetic();d["tests"][0]["observer"]=None;self.reject(d)
        d=synthetic();d["tests"][0]["browser"]=None;self.reject(d)
        d=synthetic();d["tests"][0]["private_evidence_id"]="owner-local:abc";self.reject(d)

    def test_reject_invalid_time_and_future(self):
        for bad in ("2026-10-12T10:00:00", "2026-99-99T10:00:00+01:00",
                    None, ["not","time"], "x"*300,
                    (datetime.now(timezone.utc)+timedelta(days=2)).isoformat()):
            d=synthetic();d["tests"][0]["observed_at"]=bad;self.reject(d)

    def test_reject_raw_private_paths_and_urls(self):
        for value in ("/home/person/screenshot.jpg","https://example.org/raw",
                      "owner-local:abc/path","owner-local:../secret",
                      "file:///home/person/file.png", "C:\\sensitive\\file"):
            d=synthetic()
            d["tests"][0]["private_evidence_id"]=value
            d["tests"][0]["evidence_sha256"]="a"*64
            self.reject(d)
        for bad_digest in ("ABC123","A"*64,"x"*64,"a"*63):
            d=synthetic()
            d["tests"][0]["private_evidence_id"]="owner-local:synthetic001"
            d["tests"][0]["evidence_sha256"]=bad_digest
            self.reject(d)

    def test_private_opaque_pointer_may_validate_without_trust(self):
        d=synthetic("FAIL")
        d["tests"][0]["private_evidence_id"]="owner-local:synthetic001"
        d["tests"][0]["evidence_sha256"]="a"*64
        output=validate(d)
        self.assertFalse(output["human_truth_verified"])
        self.assertNotIn("Synthetic Test Fixture",json.dumps(output))
        self.assertNotIn("synthetic001",json.dumps(output))

    def test_reject_duplicate_json_keys_and_nonstandard_literals(self):
        with self.assertRaises(ValueError):
            json.loads('{"status":"NOT_TESTED","status":"PASS"}',
                       object_pairs_hook=_no_duplicate_keys)
        with self.assertRaises(ValueError):
            json.loads('{"x": NaN}',parse_constant=_reject_nonjson_constant)


if __name__=="__main__":
    unittest.main()
