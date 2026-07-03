import json
import sys
import unittest
import urllib.parse
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(REPO_ROOT / "scripts"))

from validate_configs import (  # noqa: E402
    MIN_SECRET_LENGTH,
    ValidationError,
    _check_secrets_resolvable,
    resolve_secret_refs,
    validate_webhook_config,
)


def _command(url="https://example.com/deploy", headers=None):
    command = {
        "name": "Deploy",
        "method": "POST",
        "url": url,
    }
    if headers is not None:
        command["headers"] = headers
    return {"version": 1, "commands": [command]}


class ResolvedSecretPolicyTests(unittest.TestCase):
    """resolve_secret_refs / _check_secrets_resolvable enforce the same
    min-length and no-control-character policy as SecretProvider::resolve()
    on the firmware side, so a bad secrets.json is caught host-side too."""

    def test_short_secret_rejected(self):
        with self.assertRaisesRegex(ValidationError, "too short"):
            resolve_secret_refs({"secretRef": "token"}, {"token": "abc"})

    def test_secret_at_minimum_length_accepted(self):
        secret = "x" * MIN_SECRET_LENGTH
        self.assertEqual(resolve_secret_refs({"secretRef": "token"}, {"token": secret}), secret)

    def test_control_character_rejected(self):
        with self.assertRaisesRegex(ValidationError, "control character"):
            resolve_secret_refs({"secretRef": "token"}, {"token": "abc\r\ndef"})

    def test_multibyte_secret_measured_in_utf8_bytes_not_codepoints(self):
        # Firmware measures length via Arduino String::length() (bytes), not
        # codepoints. "秘密の値" is 4 Python characters but 12 UTF-8 bytes;
        # codepoint counting would wrongly reject it as shorter than
        # MIN_SECRET_LENGTH even though firmware accepts it.
        secret = "秘密の値"
        self.assertEqual(len(secret), 4)
        self.assertGreaterEqual(len(secret.encode("utf-8")), MIN_SECRET_LENGTH)
        self.assertEqual(resolve_secret_refs({"secretRef": "token"}, {"token": secret}), secret)

    def test_template_secret_ref_in_url_gets_same_policy(self):
        result = validate_webhook_config(_command(url="https://example.com/{{secret.token}}"))
        with self.assertRaisesRegex(ValidationError, "too short"):
            _check_secrets_resolvable(
                {"settings": {"wifi": {"ssid": "x", "password": "longenough"}},
                 "webhooks": result},
                {"token": "abc"},
            )

    def test_empty_secret_ref_name_in_url_template_rejected(self):
        with self.assertRaisesRegex(ValidationError, "secret ref must use"):
            validate_webhook_config(_command(url="https://example.com/{{secret.}}"))

    def test_empty_secret_ref_object_rejected(self):
        with self.assertRaises(ValidationError):
            resolve_secret_refs({"secretRef": ""}, {"": "irrelevant"})

    def test_whitespace_padded_secret_ref_name_rejected(self):
        with self.assertRaises(ValidationError):
            resolve_secret_refs({"secretRef": " token "}, {" token ": "longenoughvalue"})


# --- Host-side executable mirror of RedactionRegistry ---------------------
#
# There is no C++ unit-test harness in this repo (firmware behavior is
# verified via source-scan only), so the redaction ALGORITHM itself
# (register raw + percent-encoded + JSON-escaped variants, redact
# longest-variant-first, all-or-nothing capacity) is mirrored here in Python
# and exercised with real inputs. src/storage/RedactionRegistry.cpp and
# tests/test_webhook_v2_source.py pin that the C++ implementation has the
# matching structure (same constants, same all-or-nothing/longest-first
# logic); this file pins that the ALGORITHM is actually correct.


def _percent_encode(value):
    return urllib.parse.quote(value, safe="")


def _json_escape_inner(value):
    return json.dumps(value)[1:-1]


# A secret whose raw / percent-encoded / JSON-escaped forms are all
# distinct (the "/" differs under percent-encoding, the '"' differs under
# JSON escaping), so tests that expect exactly 3 registered variants aren't
# accidentally satisfied by a secret whose forms happen to collide.
_THREE_VARIANT_SECRET = 'shared/secret"value'


class MirrorRedactionRegistry:
    MAX_ENTRIES = 48
    MAX_TOTAL_BYTES = 4096

    def __init__(self):
        self._variants = []
        self._total_bytes = 0

    def register_secret(self, raw_value):
        if not raw_value:
            return "ok"
        candidates = [raw_value, _percent_encode(raw_value), _json_escape_inner(raw_value)]
        to_add = [c for c in dict.fromkeys(candidates) if c not in self._variants]
        if not to_add:
            return "ok"
        if len(self._variants) + len(to_add) > self.MAX_ENTRIES:
            return "too_many_entries"
        # Mirrors RedactionRegistry.cpp's String::length() byte accounting.
        added_bytes = sum(len(c.encode("utf-8")) for c in to_add)
        if self._total_bytes + added_bytes > self.MAX_TOTAL_BYTES:
            return "too_many_bytes"
        self._variants.extend(to_add)
        self._total_bytes += added_bytes
        return "ok"

    def redact(self, text):
        for variant in sorted(self._variants, key=len, reverse=True):
            if variant:
                text = text.replace(variant, "[REDACTED]")
        return text


class RedactionRegistryMirrorTests(unittest.TestCase):
    def test_redacts_raw_form(self):
        registry = MirrorRedactionRegistry()
        secret = "plainrawsecretvalue"
        self.assertEqual(registry.register_secret(secret), "ok")
        self.assertNotIn(secret, registry.redact(f"header value: {secret}"))

    def test_redacts_percent_encoded_form_that_differs_from_raw(self):
        registry = MirrorRedactionRegistry()
        secret = "tok3n/with+special=chars&here"
        encoded = _percent_encode(secret)
        # Sanity: this secret's percent-encoded form must actually differ
        # from the raw value, or this test would pass even if percent-encoded
        # registration were deleted entirely.
        self.assertNotEqual(encoded, secret)
        self.assertEqual(registry.register_secret(secret), "ok")

        url_leak = f"https://example.com/hook?token={encoded}"
        self.assertIn(encoded, url_leak)  # sanity: fixture contains the encoded form
        redacted = registry.redact(url_leak)
        self.assertNotIn(encoded, redacted)
        self.assertNotIn(secret, redacted)

    def test_redacts_json_escaped_form_that_differs_from_raw(self):
        registry = MirrorRedactionRegistry()
        secret = 'tok"en\\with\\backslash'
        escaped = _json_escape_inner(secret)
        # Sanity: this secret's JSON-escaped form must actually differ from
        # the raw value, or this test would pass even if JSON-escaped
        # registration were deleted entirely.
        self.assertNotEqual(escaped, secret)
        self.assertEqual(registry.register_secret(secret), "ok")

        body_leak = json.dumps({"authorization": secret})
        self.assertIn(escaped, body_leak)  # sanity: fixture contains the escaped form
        redacted = registry.redact(body_leak)
        self.assertNotIn(escaped, redacted)
        self.assertNotIn(secret, redacted)

    def test_registering_same_secret_twice_does_not_double_count_capacity(self):
        # Models a command whose header uses {"secretRef": "token"} and whose
        # url/body also references {{secret.token}} -- both surfaces resolve
        # to the same value and both call registerSecret; this must not
        # consume capacity twice or be treated as two different secrets.
        registry = MirrorRedactionRegistry()
        registry.MAX_ENTRIES = 3
        secret = _THREE_VARIANT_SECRET
        self.assertEqual(registry.register_secret(secret), "ok")
        self.assertEqual(len(registry._variants), 3)
        self.assertEqual(registry.register_secret(secret), "ok")
        self.assertEqual(len(registry._variants), 3)

    def test_registration_succeeds_at_exact_entry_capacity(self):
        registry = MirrorRedactionRegistry()
        registry.MAX_ENTRIES = 3
        self.assertEqual(registry.register_secret(_THREE_VARIANT_SECRET), "ok")
        self.assertEqual(len(registry._variants), 3)

    def test_registration_succeeds_at_exact_byte_capacity(self):
        registry = MirrorRedactionRegistry()
        secret = _THREE_VARIANT_SECRET
        variants = {secret, _percent_encode(secret), _json_escape_inner(secret)}
        registry.MAX_TOTAL_BYTES = sum(len(v.encode("utf-8")) for v in variants)
        self.assertEqual(registry.register_secret(secret), "ok")

    def test_byte_overflow_after_prior_success_leaves_existing_registration_intact(self):
        registry = MirrorRedactionRegistry()
        registry.MAX_TOTAL_BYTES = 20
        self.assertEqual(registry.register_secret("firstsecret"), "ok")
        variants_before = list(registry._variants)
        bytes_before = registry._total_bytes

        result = registry.register_secret(_THREE_VARIANT_SECRET)
        self.assertEqual(result, "too_many_bytes")
        self.assertEqual(registry._variants, variants_before)
        self.assertEqual(registry._total_bytes, bytes_before)

    def test_longest_variant_redacted_first_avoids_fragment_leak(self):
        registry = MirrorRedactionRegistry()
        short_secret = "abc123"
        long_secret = "abc123xyz789"
        self.assertEqual(registry.register_secret(short_secret), "ok")
        self.assertEqual(registry.register_secret(long_secret), "ok")

        result = registry.redact(f"long={long_secret} short={short_secret}")
        self.assertNotIn("xyz789", result)
        self.assertNotIn("abc123", result)

    def test_registration_is_all_or_nothing_on_entry_overflow(self):
        registry = MirrorRedactionRegistry()
        registry.MAX_ENTRIES = 2
        self.assertEqual(registry.register_secret("firstsecret"), "ok")
        # This secret's 3 variants would push total entries past the cap.
        result = registry.register_secret("second-secret!")
        self.assertEqual(result, "too_many_entries")

        redacted = registry.redact("first=firstsecret second=second-secret!")
        self.assertNotIn("firstsecret", redacted)
        # Not even partially protected: none of the rejected secret's
        # variants were registered, so its raw form stays fully visible
        # instead of silently leaking one encoded form.
        self.assertIn("second-secret!", redacted)

    def test_registration_is_all_or_nothing_on_byte_overflow(self):
        registry = MirrorRedactionRegistry()
        registry.MAX_TOTAL_BYTES = 10
        result = registry.register_secret("waytoolongtofitinbudget")
        self.assertEqual(result, "too_many_bytes")
        self.assertEqual(registry._variants, [])


if __name__ == "__main__":
    unittest.main()
