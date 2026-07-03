import unittest

from scripts.validate_configs import ValidationError, validate_webhook_config


def _command(**overrides):
    command = {
        "name": "Deploy",
        "method": "POST",
        "url": "https://example.com/deploy",
        "body": {"source": "cardputer"},
    }
    command.update(overrides)
    return {"version": 1, "commands": [command]}


class WebhookV1CategoryRiskTests(unittest.TestCase):
    def test_category_and_description_default_to_empty(self):
        result = validate_webhook_config(_command())
        command = result["commands"][0]
        self.assertEqual(command["category"], "")
        self.assertEqual(command["description"], "")
        self.assertEqual(command["risk"], "low")
        self.assertFalse(command["requirePreview"])

    def test_category_and_description_round_trip(self):
        result = validate_webhook_config(
            _command(category="deploy", description="Trigger a deploy")
        )
        command = result["commands"][0]
        self.assertEqual(command["category"], "deploy")
        self.assertEqual(command["description"], "Trigger a deploy")

    def test_rejects_empty_category_string(self):
        with self.assertRaisesRegex(ValidationError, "category"):
            validate_webhook_config(_command(category=""))

    def test_rejects_unknown_risk_level(self):
        with self.assertRaisesRegex(ValidationError, "risk must be low, medium, or high"):
            validate_webhook_config(_command(risk="critical"))

    def test_risk_high_requires_confirm_and_require_preview(self):
        with self.assertRaisesRegex(
            ValidationError, "risk:high requires confirm:true and requirePreview:true"
        ):
            validate_webhook_config(_command(risk="high"))

    def test_risk_high_requires_confirm_even_with_preview(self):
        with self.assertRaisesRegex(
            ValidationError, "risk:high requires confirm:true and requirePreview:true"
        ):
            validate_webhook_config(_command(risk="high", requirePreview=True))

    def test_risk_high_requires_preview_even_with_confirm(self):
        # Covers the other half of the AND: confirm alone must not be enough.
        with self.assertRaisesRegex(
            ValidationError, "risk:high requires confirm:true and requirePreview:true"
        ):
            validate_webhook_config(_command(risk="high", confirm=True))

    def test_risk_high_accepted_with_confirm_and_preview(self):
        result = validate_webhook_config(
            _command(risk="high", confirm=True, requirePreview=True)
        )
        self.assertEqual(result["commands"][0]["risk"], "high")


class WebhookV1InputFieldTests(unittest.TestCase):
    def test_text_field_defaults(self):
        result = validate_webhook_config(
            _command(
                inputs=[{"key": "env", "kind": "text", "label": "Environment"}]
            )
        )
        field = result["commands"][0]["inputs"][0]
        self.assertEqual(field["maxLength"], 96)
        self.assertTrue(field["required"])
        self.assertEqual(field["default"], "")

    def test_text_default_exceeding_max_length_rejected(self):
        with self.assertRaisesRegex(ValidationError, "default exceeds maxLength"):
            validate_webhook_config(
                _command(
                    inputs=[
                        {
                            "key": "reason",
                            "kind": "text",
                            "label": "Reason",
                            "maxLength": 5,
                            "default": "too-long-for-this-field",
                        }
                    ]
                )
            )

    def test_choice_default_not_in_choices_rejected(self):
        with self.assertRaisesRegex(ValidationError, "default must be one of choices"):
            validate_webhook_config(
                _command(
                    inputs=[
                        {
                            "key": "env",
                            "kind": "choice",
                            "label": "Environment",
                            "choices": ["staging", "prod"],
                            "default": "dev",
                        }
                    ]
                )
            )

    def test_boolean_default_must_be_true_or_false_string(self):
        with self.assertRaisesRegex(ValidationError, "default must be true or false"):
            validate_webhook_config(
                _command(
                    inputs=[
                        {
                            "key": "notify",
                            "kind": "boolean",
                            "label": "Notify?",
                            "default": "yes",
                        }
                    ]
                )
            )

    def test_confirmation_rejects_any_default(self):
        with self.assertRaisesRegex(
            ValidationError, "default is not valid for kind confirmation"
        ):
            validate_webhook_config(
                _command(
                    inputs=[
                        {
                            "key": "confirmed",
                            "kind": "confirmation",
                            "label": "Confirm",
                            "default": "true",
                        }
                    ]
                )
            )

    def test_rejects_key_with_uppercase(self):
        with self.assertRaisesRegex(ValidationError, r"\.key must start with a-z"):
            validate_webhook_config(
                _command(inputs=[{"key": "Env", "kind": "text", "label": "Environment"}])
            )

    def test_rejects_unknown_kind(self):
        with self.assertRaisesRegex(ValidationError, "kind must be"):
            validate_webhook_config(
                _command(inputs=[{"key": "env", "kind": "number", "label": "Environment"}])
            )

    def test_choice_requires_two_to_eight_entries(self):
        with self.assertRaisesRegex(ValidationError, "choices must have 2 to 8 entries"):
            validate_webhook_config(
                _command(
                    inputs=[
                        {
                            "key": "env",
                            "kind": "choice",
                            "label": "Environment",
                            "choices": ["only-one"],
                        }
                    ]
                )
            )

    def test_choice_accepts_valid_entries(self):
        result = validate_webhook_config(
            _command(
                inputs=[
                    {
                        "key": "env",
                        "kind": "choice",
                        "label": "Environment",
                        "choices": ["staging", "prod"],
                    }
                ]
            )
        )
        self.assertEqual(result["commands"][0]["inputs"][0]["choices"], ["staging", "prod"])

    def test_choice_accepts_upper_bound_of_eight_entries(self):
        choices = [f"opt{i}" for i in range(8)]
        result = validate_webhook_config(
            _command(
                inputs=[
                    {"key": "env", "kind": "choice", "label": "Environment", "choices": choices}
                ]
            )
        )
        self.assertEqual(result["commands"][0]["inputs"][0]["choices"], choices)

    def test_choice_rejects_nine_entries(self):
        choices = [f"opt{i}" for i in range(9)]
        with self.assertRaisesRegex(ValidationError, "choices must have 2 to 8 entries"):
            validate_webhook_config(
                _command(
                    inputs=[
                        {
                            "key": "env",
                            "kind": "choice",
                            "label": "Environment",
                            "choices": choices,
                        }
                    ]
                )
            )

    def test_choices_rejected_outside_choice_kind(self):
        with self.assertRaisesRegex(ValidationError, "choices is only valid for kind choice"):
            validate_webhook_config(
                _command(
                    inputs=[
                        {
                            "key": "env",
                            "kind": "text",
                            "label": "Environment",
                            "choices": ["a", "b"],
                        }
                    ]
                )
            )

    def test_max_length_rejected_outside_text_kind(self):
        with self.assertRaisesRegex(ValidationError, "maxLength is only valid for kind text"):
            validate_webhook_config(
                _command(
                    inputs=[
                        {
                            "key": "confirmed",
                            "kind": "confirmation",
                            "label": "Confirm",
                            "maxLength": 10,
                        }
                    ]
                )
            )

    def test_max_length_out_of_range_rejected(self):
        with self.assertRaisesRegex(ValidationError, "maxLength must be between 1 and 128"):
            validate_webhook_config(
                _command(
                    inputs=[
                        {
                            "key": "reason",
                            "kind": "text",
                            "label": "Reason",
                            "maxLength": 500,
                        }
                    ]
                )
            )

    def test_max_length_accepts_lower_and_upper_bounds(self):
        for bound in (1, 128):
            result = validate_webhook_config(
                _command(
                    inputs=[
                        {
                            "key": "reason",
                            "kind": "text",
                            "label": "Reason",
                            "maxLength": bound,
                        }
                    ]
                )
            )
            self.assertEqual(result["commands"][0]["inputs"][0]["maxLength"], bound)

    def test_max_length_rejects_zero_and_one_past_upper_bound(self):
        for bound in (0, 129):
            with self.assertRaisesRegex(
                ValidationError, "maxLength must be between 1 and 128"
            ):
                validate_webhook_config(
                    _command(
                        inputs=[
                            {
                                "key": "reason",
                                "kind": "text",
                                "label": "Reason",
                                "maxLength": bound,
                            }
                        ]
                    )
                )

    def test_max_length_rejects_boolean(self):
        # bool is an int subclass in Python; must not silently pass as 1.
        with self.assertRaisesRegex(ValidationError, "maxLength must be between 1 and 128"):
            validate_webhook_config(
                _command(
                    inputs=[
                        {
                            "key": "reason",
                            "kind": "text",
                            "label": "Reason",
                            "maxLength": True,
                        }
                    ]
                )
            )

    def test_rejects_duplicate_keys(self):
        with self.assertRaisesRegex(ValidationError, "duplicate key 'env'"):
            validate_webhook_config(
                _command(
                    inputs=[
                        {"key": "env", "kind": "text", "label": "Environment"},
                        {"key": "env", "kind": "text", "label": "Environment again"},
                    ]
                )
            )


class WebhookV1PlaceholderTests(unittest.TestCase):
    def _inputs(self):
        return [{"key": "env", "kind": "text", "label": "Environment"}]

    def test_url_placeholder_after_host_accepted(self):
        result = validate_webhook_config(
            _command(url="https://example.com/deploy/{{input.env}}", inputs=self._inputs())
        )
        self.assertEqual(result["commands"][0]["url"], "https://example.com/deploy/{{input.env}}")

    def test_url_placeholder_in_host_rejected(self):
        with self.assertRaisesRegex(ValidationError, "placeholder not allowed before the host"):
            validate_webhook_config(
                _command(
                    url="https://{{input.env}}.example.com/deploy", inputs=self._inputs()
                )
            )

    def test_url_placeholder_unknown_namespace_rejected(self):
        with self.assertRaisesRegex(ValidationError, "reserved or unknown"):
            validate_webhook_config(
                _command(url="https://example.com/deploy/{{foo.token}}", inputs=self._inputs())
            )

    def test_url_placeholder_secret_namespace_accepted(self):
        # {{secret.<ref>}} is now implemented for url/body (PR2); only ref
        # syntax is checked here, resolution happens at render time.
        result = validate_webhook_config(
            _command(url="https://example.com/deploy/{{secret.token}}", inputs=self._inputs())
        )
        self.assertEqual(
            result["commands"][0]["url"], "https://example.com/deploy/{{secret.token}}"
        )

    def test_url_placeholder_secret_ref_bad_syntax_rejected(self):
        with self.assertRaisesRegex(ValidationError, "secret ref must use"):
            validate_webhook_config(
                _command(
                    url="https://example.com/deploy/{{secret.bad ref}}", inputs=self._inputs()
                )
            )

    def test_url_placeholder_undefined_key_rejected(self):
        with self.assertRaisesRegex(ValidationError, "undefined input 'missing'"):
            validate_webhook_config(
                _command(
                    url="https://example.com/deploy/{{input.missing}}", inputs=self._inputs()
                )
            )

    def test_url_unterminated_placeholder_rejected(self):
        with self.assertRaisesRegex(ValidationError, "unterminated placeholder"):
            validate_webhook_config(
                _command(url="https://example.com/deploy/{{input.env", inputs=self._inputs())
            )

    def test_header_placeholder_accepted(self):
        result = validate_webhook_config(
            _command(
                headers={"X-Env": "{{input.env}}"},
                inputs=self._inputs(),
            )
        )
        self.assertEqual(result["commands"][0]["headers"]["X-Env"], "{{input.env}}")

    def test_header_placeholder_rejected_on_secret_ref(self):
        # secretRef values are opaque and never scanned for placeholders.
        result = validate_webhook_config(
            _command(
                headers={"Authorization": {"secretRef": "token"}},
            )
        )
        self.assertEqual(result["commands"][0]["headers"]["Authorization"], {"secretRef": "token"})

    def test_body_whole_value_placeholder_accepted(self):
        result = validate_webhook_config(
            _command(body={"env": "{{input.env}}"}, inputs=self._inputs())
        )
        self.assertEqual(result["commands"][0]["body"], {"env": "{{input.env}}"})

    def test_body_partial_placeholder_rejected(self):
        with self.assertRaisesRegex(
            ValidationError, "placeholder must be the entire JSON string value"
        ):
            validate_webhook_config(
                _command(
                    body={"env": "prefix-{{input.env}}-suffix"}, inputs=self._inputs()
                )
            )

    def test_body_placeholder_in_nested_list_accepted(self):
        result = validate_webhook_config(
            _command(body={"tags": ["static", "{{input.env}}"]}, inputs=self._inputs())
        )
        self.assertEqual(result["commands"][0]["body"]["tags"][1], "{{input.env}}")

    def test_body_placeholder_as_object_key_rejected(self):
        # A typed value must never be able to rename a JSON body field.
        with self.assertRaisesRegex(
            ValidationError, "placeholder must be a value, not an object key"
        ):
            validate_webhook_config(
                _command(body={"{{input.env}}": "fixed"}, inputs=self._inputs())
            )

    def test_body_placeholder_like_key_variants_rejected(self):
        # Keys that merely contain "{{"/"}}" (not an exact whole-key match)
        # must still be rejected, not silently treated as an opaque literal.
        for bad_key in ["prefix{{input.env}}", "{{input.env}}suffix", "{{input.env"]:
            with self.assertRaisesRegex(
                ValidationError, "placeholder must be a value, not an object key"
            ):
                validate_webhook_config(
                    _command(body={bad_key: "fixed"}, inputs=self._inputs())
                )


if __name__ == "__main__":
    unittest.main()
