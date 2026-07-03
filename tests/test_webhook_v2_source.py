import unittest
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]


def _read(*parts):
    return (REPO_ROOT.joinpath(*parts)).read_text()


class SecretProviderAbstractionSourceTests(unittest.TestCase):
    # Supersedes the pre-PR2 provisioning source tests: the SD-direct
    # SecretStore class was replaced by an abstract SecretProvider interface
    # plus an SdSecretProvider implementation, so every call site now
    # depends on the interface instead of a concrete storage backend.

    def test_secret_provider_interface_declares_policy_contract(self):
        header = _read("src", "storage", "SecretProvider.h")
        for text in [
            "class SecretProvider",
            "static constexpr size_t kMinSecretLength = 6",
            "virtual bool resolve(const String& ref, String& value) = 0;",
            "virtual bool exists(const String& ref) = 0;",
            "bool isValidSecretRefName(const String& ref);",
        ]:
            self.assertIn(text, header)

    def test_sd_secret_provider_enforces_min_length_and_control_chars(self):
        source = _read("src", "storage", "SdSecretProvider.cpp")
        header = _read("src", "storage", "SdSecretProvider.h")
        self.assertIn("class SdSecretProvider : public SecretProvider", header)
        self.assertIn('SD.open("/secrets.json"', source)
        self.assertIn("length < kMinSecretLength", source)
        self.assertIn("hasControlCharacter(candidate)", source)

    def test_config_loader_depends_on_provider_interface_not_sd_store(self):
        header = _read("src", "storage", "ConfigLoader.h")
        source = _read("src", "storage", "ConfigLoader.cpp")
        self.assertIn('#include "storage/SecretProvider.h"', header)
        self.assertNotIn("SecretStore", header)
        self.assertIn("SecretProvider* secrets = nullptr", header)
        self.assertNotIn("SecretStore", source)

    def test_app_context_wires_provider_interface(self):
        context = _read("src", "launcher", "AppContext.h")
        self.assertIn('#include "storage/SecretProvider.h"', context)
        self.assertIn("SecretProvider& secrets;", context)
        self.assertNotIn("SecretStore", context)

    def test_main_instantiates_sd_backed_provider(self):
        source = _read("src", "main.cpp")
        self.assertIn("SdSecretProvider secrets;", source)


class ResolveSecretOrLiteralSourceTests(unittest.TestCase):
    def test_config_loader_shares_header_and_wifi_password_resolution(self):
        source = _read("src", "storage", "ConfigLoader.cpp")
        self.assertIn("bool resolveSecretOrLiteral(", source)
        # Header values and wifi.password both route through the same
        # resolver, so both get the same {"secretRef": ...} object syntax
        # and the same SecretProvider policy checks for free.
        self.assertIn(
            "resolveSecretOrLiteral(kv.value(), secrets, path + \".headers.\" + key, "
            "header.value,",
            source,
        )
        self.assertIn(
            'resolveSecretOrLiteral(doc["wifi"]["password"], secrets, '
            '"settings.wifi.password",',
            source,
        )

    def test_wifi_password_keeps_non_empty_contract_for_literal_values(self):
        source = _read("src", "storage", "ConfigLoader.cpp")
        self.assertIn("if (!passwordIsSecret) {", source)
        self.assertIn("settings.wifi.password must be a non-empty string", source)


class CommandTemplateSecretSourceTests(unittest.TestCase):
    def test_command_template_resolves_secret_namespace_for_url_and_body(self):
        header = _read("src", "network", "CommandTemplate.h")
        source = _read("src", "network", "CommandTemplate.cpp")
        self.assertIn("SecretProvider* secrets = nullptr", header)
        self.assertIn("std::vector<String>* resolvedSecrets = nullptr", header)
        self.assertIn('if (ns == "secret") {', source)
        self.assertIn("resolvedSecrets->push_back(value)", source)

    def test_render_headers_does_not_accept_secret_namespace(self):
        # Headers keep the pre-existing {"secretRef": ...} object syntax
        # (resolved once at config load time); the {{secret.x}} string
        # template is only for url/body.
        source = _read("src", "network", "CommandTemplate.cpp")
        render_headers_start = source.index("bool renderHeaders(")
        render_body_start = source.index("bool renderBody(")
        render_headers_body = source[render_headers_start:render_body_start]
        self.assertNotIn("resolvePlaceholderValue", render_headers_body)


class RedactionRegistrySourceTests(unittest.TestCase):
    def test_registry_declares_capacity_limits_and_fail_closed_result(self):
        header = _read("src", "storage", "RedactionRegistry.h")
        for text in [
            "enum class RegisterResult { Ok, TooManyEntries, TooManyBytes };",
            "static constexpr size_t kMaxEntries = 48;",
            "static constexpr size_t kMaxTotalBytes = 4096;",
            "RegisterResult registerSecret(const String& rawValue);",
            "String redact(const String& text) const;",
        ]:
            self.assertIn(text, header)

    def test_register_secret_is_all_or_nothing(self):
        # A secret whose variants cannot all fit must not be partially
        # registered -- that would leave one encoded form unprotected.
        source = _read("src", "storage", "RedactionRegistry.cpp")
        self.assertIn("if (toAdd.empty())", source)
        self.assertIn("if (variants_.size() + toAdd.size() > kMaxEntries)", source)
        self.assertIn("if (totalBytes_ + addedBytes > kMaxTotalBytes)", source)

    def test_redact_matches_longest_variant_first(self):
        source = _read("src", "storage", "RedactionRegistry.cpp")
        self.assertIn("a.length() > b.length()", source)

    def test_registry_registers_percent_encoded_and_json_escaped_variants(self):
        source = _read("src", "storage", "RedactionRegistry.cpp")
        self.assertIn("percentEncode(rawValue)", source)
        self.assertIn("escapeJsonStringInner(rawValue)", source)


class LogStoreTwoStageRedactionSourceTests(unittest.TestCase):
    def test_redact_consults_registry_before_keyword_fallback(self):
        header = _read("src", "storage", "LogStore.h")
        source = _read("src", "storage", "LogStore.cpp")
        self.assertIn(
            "String redact(const String& value, const RedactionRegistry* registry = nullptr)",
            header,
        )
        self.assertIn("registry ? registry->redact(value) : value", source)
        self.assertIn("if (!hasSecretWord(lowered))", source)


class WebhookLauncherRedactionWiringSourceTests(unittest.TestCase):
    def test_finish_input_collection_registers_secrets_and_fails_closed_on_overflow(self):
        source = _read("src", "apps", "WebhookLauncherApp.cpp")
        self.assertIn("registerPendingSecretsForRedaction(command, resolvedSecrets)", source)
        self.assertIn("Too many secrets in this command", source)
        # Both failure paths (render error, registry overflow) must clear the
        # partially-built secret state before returning, not just on abort.
        self.assertIn("failWithToast(pendingRequest_.error);", source)
        self.assertIn(
            'failWithToast("Too many secrets in this command");\n    return;', source
        )

    def test_clear_pending_secrets_wipes_registry_cache_and_rendered_request(self):
        # A prior version cleared pendingRedaction_/redactedPreviewUrl_ but
        # left pendingRequest_ (which also holds resolved secret material:
        # percent-encoded in the url, escaped in the body, raw in sensitive
        # headers) resident until the next command overwrote it. Caught in
        # Phase 8 QA/security review.
        source = _read("src", "apps", "WebhookLauncherApp.cpp")
        self.assertIn(
            "void WebhookLauncherApp::clearPendingSecrets() {\n"
            "  pendingRedaction_ = RedactionRegistry();\n"
            '  redactedPreviewUrl_ = "";',
            source,
        )
        self.assertIn("pendingRequest_ = RenderedRequest();", source)

    def test_every_execute_and_failure_exit_clears_pending_secrets(self):
        source = _read("src", "apps", "WebhookLauncherApp.cpp")
        execute_start = source.index("void WebhookLauncherApp::execute(")
        execute_body = source[execute_start:]
        # abortCommand() and both finishInputCollection() failure paths (via
        # failWithToast) plus all 3 exit points inside execute() must clear
        # pending secret state -- 5 call sites total.
        self.assertIn(
            "void WebhookLauncherApp::abortCommand(const char* reason) {\n"
            "  stage_ = Stage::List;\n"
            "  status_ = reason;\n"
            "  clearPendingSecrets();",
            source,
        )
        self.assertIn(
            "stage_ = Stage::List;\n    clearPendingSecrets();\n  };",
            source,
        )
        self.assertEqual(execute_body.count("clearPendingSecrets();"), 3)

    def test_preview_and_confirm_screens_redact_the_resolved_url(self):
        # The redacted URL is computed once (not on every render() tick) and
        # cached in redactedPreviewUrl_; both the Preview and AwaitingConfirm
        # screens must display that cached value, not re-run redact().
        source = _read("src", "apps", "WebhookLauncherApp.cpp")
        self.assertIn("redactedPreviewUrl_ = pendingRedaction_.redact(pendingRequest_.url);",
                      source)
        self.assertEqual(source.count("previewUrlLine(redactedPreviewUrl_)"), 2)

    def test_execute_redacts_response_preview_before_display_and_logging(self):
        source = _read("src", "apps", "WebhookLauncherApp.cpp")
        self.assertIn("preview_ = pendingRedaction_.redact(response.preview);", source)
        self.assertIn("appendRequest(", source)
        self.assertIn("&pendingRedaction_", source)

    def test_execute_passes_provider_to_wifi_password_resolution(self):
        source = _read("src", "apps", "WebhookLauncherApp.cpp")
        self.assertIn("ctx.config.loadWifi(settings, &ctx.secrets)", source)


if __name__ == "__main__":
    unittest.main()
