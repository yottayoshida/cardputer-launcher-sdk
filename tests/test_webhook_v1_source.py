import unittest
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]


def _read(*parts):
    return (REPO_ROOT.joinpath(*parts)).read_text()


class KeyboardModeSourceTests(unittest.TestCase):
    def test_input_action_and_mode_declared(self):
        header = _read("src", "input", "Keyboard.h")
        for text in ["Left", "Right", "Clear", "ToggleSearch", "enum class InputMode"]:
            self.assertIn(text, header)
        self.assertIn("InputEvent poll(InputMode mode", header)

    def test_poll_uses_fn_layer_fields_before_word_loop(self):
        source = _read("src", "input", "Keyboard.cpp")
        for text in [
            "status.esc",
            "status.tab",
            "status.up",
            "status.down",
            "status.left",
            "status.right",
            "status.del",
            "status.backspace",
        ]:
            self.assertIn(text, source)
        # w/s/y/n legacy shortcuts must stay gated behind Navigation mode so
        # TextEntry lets them through as literal characters.
        self.assertIn("mode == InputMode::Navigation", source)

    def test_launcher_defers_back_to_text_entry_apps(self):
        source = _read("src", "launcher", "Launcher.cpp")
        self.assertIn("inputMode(ctx) == InputMode::TextEntry", source)
        self.assertIn("filterIndices", source)

    def test_launcher_also_defers_back_when_app_owns_it(self):
        # ownsBack() is distinct from TextEntry so a multi-step app (preview,
        # confirm, choice/boolean fields) can claim Back without needing raw
        # character passthrough.
        header = _read("src", "launcher", "App.h")
        source = _read("src", "launcher", "Launcher.cpp")
        self.assertIn("virtual bool ownsBack(", header)
        self.assertIn("activeApp_->ownsBack(ctx)", source)


class TextInputSourceTests(unittest.TestCase):
    def test_text_input_declares_cursor_and_window_api(self):
        header = _read("src", "ui", "TextInput.h")
        for text in ["setMaxLength", "seed", "cursor() const", "renderWindow"]:
            self.assertIn(text, header)

    def test_text_input_handles_clear_and_cursor_movement(self):
        source = _read("src", "ui", "TextInput.cpp")
        for text in ["InputAction::Clear", "InputAction::Left", "InputAction::Right"]:
            self.assertIn(text, source)


class ConfigLoaderV1SourceTests(unittest.TestCase):
    def test_webhook_command_declares_v1_fields(self):
        header = _read("src", "storage", "ConfigLoader.h")
        for text in [
            "enum class RiskLevel",
            "struct InputField",
            "bool sensitive = false",
            "String category;",
            "RiskLevel risk = RiskLevel::Low;",
            "bool requirePreview = false;",
            "std::vector<InputField> inputs;",
        ]:
            self.assertIn(text, header)

    def test_config_loader_enforces_risk_high_gate(self):
        source = _read("src", "storage", "ConfigLoader.cpp")
        self.assertIn("RiskLevel::High && !(command.confirm && command.requirePreview)", source)

    def test_config_loader_validates_placeholders_before_and_after_authority(self):
        source = _read("src", "storage", "ConfigLoader.cpp")
        for text in [
            "validatePlaceholderToken",
            "validateEmbeddedPlaceholders",
            "validateUrlPlaceholders",
            "validateBodyPlaceholders",
            'ns != "input"',
            "placeholder not allowed before the host",
            "placeholder must be the entire JSON string value",
        ]:
            self.assertIn(text, source)

    def test_scheme_comparison_is_case_insensitive(self):
        source = _read("src", "storage", "ConfigLoader.cpp")
        self.assertIn("startsWithScheme", source)
        self.assertIn("equalsIgnoreCase(scheme)", source)

    def test_revalidate_resolved_url_is_exposed(self):
        header = _read("src", "storage", "ConfigLoader.h")
        source = _read("src", "storage", "ConfigLoader.cpp")
        self.assertIn("bool revalidateResolvedUrl(", header)
        self.assertIn("still contains an unresolved placeholder", source)

    def test_body_placeholder_rejects_object_key_position(self):
        # A typed value must never be able to rename a JSON body field.
        source = _read("src", "storage", "ConfigLoader.cpp")
        self.assertIn("placeholder must be a value, not an object key", source)
        self.assertIn("bodyJson[afterQuote] == ':'", source)

    def test_input_field_default_validated_against_kind_constraints(self):
        # A bad default (too long, not a declared choice, not a real
        # boolean) must fail loudly at load time, not be silently clamped.
        source = _read("src", "storage", "ConfigLoader.cpp")
        for text in [
            "field.defaultValue.length() > field.maxLength",
            "must be one of choices",
            'field.defaultValue != "true" && field.defaultValue != "false"',
            "is not valid for kind confirmation",
        ]:
            self.assertIn(text, source)


class HttpClientSchemePolicySourceTests(unittest.TestCase):
    def test_execution_policy_matches_config_loader_scheme_case_handling(self):
        # ConfigLoader::validateHttpsUrl accepts mixed-case schemes; the
        # request-time policy gate must agree or a validly-loaded command
        # fails at execute time with "URL policy blocked".
        source = _read("src", "network", "HttpClient.cpp")
        self.assertIn("startsWithScheme", source)
        self.assertIn("equalsIgnoreCase(scheme)", source)


class CommandTemplateSourceTests(unittest.TestCase):
    def test_command_template_declares_binding_and_rendered_request(self):
        header = _read("src", "network", "CommandTemplate.h")
        for text in ["struct TemplateBinding", "struct RenderedRequest", "renderCommandTemplate"]:
            self.assertIn(text, header)

    def test_command_template_percent_encodes_url_and_escapes_json(self):
        source = _read("src", "network", "CommandTemplate.cpp")
        self.assertIn("percentEncode", source)
        self.assertIn("escapeJsonString", source)
        self.assertIn("revalidateResolvedUrl(url, command.allowLocalHttp", source)

    def test_command_template_converts_boolean_inputs_to_json_literals(self):
        source = _read("src", "network", "CommandTemplate.cpp")
        self.assertIn("InputField::Kind::Boolean", source)
        self.assertIn('(*boundValue == "true") ? "true" : "false"', source)

    def test_command_template_rejects_header_line_breaks(self):
        source = _read("src", "network", "CommandTemplate.cpp")
        self.assertIn("cannot contain a line break", source)


class WebhookLauncherAppStateMachineSourceTests(unittest.TestCase):
    def test_declares_stage_enum_and_field_collection_state(self):
        header = _read("src", "apps", "WebhookLauncherApp.h")
        for text in [
            "enum class Stage { List, CollectingInput, Preview, AwaitingConfirm };",
            "std::vector<TemplateBinding> bindings_;",
            "RenderedRequest pendingRequest_;",
        ]:
            self.assertIn(text, header)

    def test_require_preview_gates_execute_behind_preview_stage(self):
        source = _read("src", "apps", "WebhookLauncherApp.cpp")
        self.assertIn("if (command.requirePreview) {", source)
        self.assertIn("stage_ = Stage::Preview;", source)

    def test_preview_masks_sensitive_header_values(self):
        source = _read("src", "apps", "WebhookLauncherApp.cpp")
        self.assertIn('header.sensitive ? "***" : header.value', source)

    def test_execute_logs_rendered_url_not_template(self):
        source = _read("src", "apps", "WebhookLauncherApp.cpp")
        self.assertIn("command.name, command.method, rendered.url", source)

    def test_owns_back_claims_back_for_every_non_list_stage(self):
        source = _read("src", "apps", "WebhookLauncherApp.cpp")
        self.assertIn("bool WebhookLauncherApp::ownsBack(const AppContext&) const {"
                      " return stage_ != Stage::List; }", source)

    def test_cancel_always_aborts_and_boolean_uses_arrow_toggle_only(self):
        # In Navigation mode the keyboard driver turns 'y'/'n' into
        # InputAction::Confirm/Cancel before they ever reach onInput() as
        # Character events. Cancel ('n') must mean "abort" consistently
        # across every field kind (a UX finding: Boolean previously
        # overloaded Cancel to also mean "set false", which is inconsistent
        # with every other field kind and error-prone). Boolean is toggled
        # only via the arrow keys.
        source = _read("src", "apps", "WebhookLauncherApp.cpp")
        self.assertIn(
            "if (event.action == InputAction::Cancel) {\n"
            '    abortCommand("Canceled");\n'
            "    return;\n"
            "  }",
            source,
        )
        self.assertIn(
            "case InputField::Kind::Boolean:\n"
            "      if (event.action == InputAction::Select) {",
            source,
        )
        self.assertNotIn("event.character == 'y'", source)
        self.assertNotIn("event.character == 'n'", source)
        self.assertNotIn("boolValue_ = true;", source)
        self.assertNotIn("boolValue_ = false;", source)


if __name__ == "__main__":
    unittest.main()
