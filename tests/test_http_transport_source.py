import unittest
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]


class HttpTransportSourceTests(unittest.TestCase):
    def test_config_loader_parses_allow_local_http_flag(self):
        header = (REPO_ROOT / "src/storage/ConfigLoader.h").read_text()
        source = (REPO_ROOT / "src/storage/ConfigLoader.cpp").read_text()
        app = (REPO_ROOT / "src/apps/WebhookLauncherApp.cpp").read_text()

        self.assertIn("bool allowLocalHttp", header)
        self.assertIn('item["allowLocalHttp"]', source)
        self.assertIn("request.allowLocalHttp = command.allowLocalHttp", app)

    def test_http_client_declares_error_kind_and_limits(self):
        header = (REPO_ROOT / "src/network/HttpClient.h").read_text()
        source = (REPO_ROOT / "src/network/HttpClient.cpp").read_text()

        for text in [
            "enum class HttpErrorKind",
            "Policy",
            "Limit",
            "Timeout",
            "HttpStatus",
            "HttpErrorKind errorKind",
            "bool allowLocalHttp",
        ]:
            self.assertIn(text, header)

        for text in [
            "kConnectTimeoutMs",
            "kReadTimeoutMs",
            "kMaxRequestBodyBytes",
            "kMaxResponsePreviewBytes",
            "kGetRetryCount",
            "http.setConnectTimeout(kConnectTimeoutMs)",
            "http.setTimeout(kReadTimeoutMs)",
        ]:
            self.assertIn(text, source)

    def test_http_client_enforces_policy_and_get_only_retry(self):
        source = (REPO_ROOT / "src/network/HttpClient.cpp").read_text()

        for text in [
            "isAllowedUrl(request.url, request.allowLocalHttp)",
            "request.body.length() > kMaxRequestBodyBytes",
            "request.method == \"GET\" ? kGetRetryCount : 0",
            "attempt <= retries",
            "HttpErrorKind::Policy",
            "HttpErrorKind::Limit",
            "HttpErrorKind::Network",
            "HttpErrorKind::Timeout",
            "HttpErrorKind::HttpStatus",
        ]:
            self.assertIn(text, source)


if __name__ == "__main__":
    unittest.main()
