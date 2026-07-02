# Issue 19 HTTP Transport Hardening Design

## Goal

Harden Webhook Launcher transport behavior so network requests have explicit safety policy, bounded runtime, bounded payloads, and clearer failure categories before v1.0.

## Scope

- Keep HTTPS as the default for command URLs.
- Add a per-command `allowLocalHttp` flag for local development only.
- Permit `http://localhost`, `http://127.0.0.1`, and `http://[::1]` only when `allowLocalHttp` is true.
- Add firmware HTTP connect/read timeouts.
- Add a single bounded retry only for GET transport failures.
- Reject oversized request bodies before sending.
- Keep response previews bounded before display or logging.
- Add a small transport error classification for policy, limit, network, timeout, and HTTP-status failures.
- Document TLS limitations and future pinning options honestly.

## Non-Goals

- No certificate pinning implementation.
- No custom CA bundle or trust-anchor provisioning.
- No hardware Wi-Fi validation claim.
- No POST retry.
- No dependency on the issue #10 `secretRef` branch.

## Design

Webhook command config gains optional:

```json
{
  "allowLocalHttp": true
}
```

If absent or false, URLs must be HTTPS. If true, the only accepted HTTP hosts are `localhost`, `127.0.0.1`, and `[::1]`. This makes local webhook testing possible without allowing accidental plaintext production endpoints.

`HttpClient` enforces the same policy at send time through `HttpRequest::allowLocalHttp`. It also sets connect and read timeouts on Arduino `HTTPClient`, rejects bodies over 2048 bytes, bounds response preview at 160 bytes, and retries one time only for GET transport failures where the HTTP library returns a non-positive status.

The firmware response struct exposes a compact `HttpErrorKind` enum so UI/log layers can distinguish policy, limit, network, timeout, and HTTP status classes without parsing arbitrary strings.

## Acceptance Mapping

- HTTPS default and local-development exception are tested in host validation and firmware source tests.
- Timeouts and bounded GET retry are enforced in `src/network/HttpClient.cpp`.
- Request body and response preview limits are enforced and documented.
- `docs/SECURITY.md` documents TLS limitations, local HTTP exceptions, and future pinning options.
