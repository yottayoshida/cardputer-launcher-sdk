# UX

## Principles

- Keyboard first.
- One decision per screen.
- Short labels over explanatory paragraphs.
- Visible failure states.
- No hidden network action for commands marked `confirm: true`.

## Navigation

Default keys:

- `w` or `k`: move up.
- `s` or `j`: move down.
- Enter: select.
- Delete/backspace-style key: back.
- `y`: confirm when prompted.
- `n`: cancel when prompted.

## Screen Types

Launcher:

```text
Launcher      SD:ok
> Webhook Launcher
  Log Viewer
  Settings/About
```

Webhook command list:

```text
Webhooks      3 cmd
> Deploy Preview
  Start Backup
  Ping Healthcheck
```

Confirmation:

```text
Deploy Preview
POST example.com

Y confirm / N cancel
```

Result:

```text
Deploy Preview
HTTP 200
{"ok":true}
```

## Error States

- No SD card: show `SD unavailable`.
- Missing settings: show `Wi-Fi config missing`.
- Config parse failure: show `Config parse error`.
- Wi-Fi connection failure: show `Wi-Fi connect failed`.
- HTTP request failure: show `HTTP error`.
- Success: show status code and short response preview.

## Future UX

Search, QR handoff, command categories, richer forms, and app-specific status icons are future work after the v0.1 interaction model is stable.

