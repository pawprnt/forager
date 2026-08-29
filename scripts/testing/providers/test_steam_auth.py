import json
import urllib.parse
from http.cookiejar import Cookie, CookieJar

import pytest

from forager.providers.steam import auth as steam_auth
from forager.providers.steam.auth import SteamAuthError


class FakeResp:
    def __init__(self, text="", status=200):
        self._text = text
        self.status = status

    def read(self):
        return self._text.encode("utf-8") if isinstance(self._text, str) else self._text

    def close(self):
        return None


def _json_resp(data, status=200):
    return FakeResp(json.dumps({"response": data}) if isinstance(data, dict) else json.dumps(data), status)


def _make_cookie(jar, name, value, domain="steamcommunity.com"):
    cookie = Cookie(
        version=0, name=name, value=value, port=None, port_specified=False,
        domain=domain, domain_specified=True, domain_initial_dot=False,
        path="/", path_specified=True, secure=True, expires=None, discard=False,
        comment=None, comment_url=None, rest=None, rfc2109=False,
    )
    jar.set_cookie(cookie)


# --------------------------------------------------------------------------
# QR
# --------------------------------------------------------------------------


def test_start_qr_session(monkeypatch):
    calls = {}

    def fake_urlopen(req, timeout=0):
        calls["method"] = req.method
        calls["url"] = req.full_url
        calls["body"] = json.loads(req.data)
        calls["timeout"] = timeout
        return _json_resp({
            "client_id": "12345",
            "request_id": "req-1",
            "interval": 5,
            "challenge_url": "https://s.team/q/1/12345",
            "allowed_confirmations": [{"confirmation_type": 4}, {"confirmation_type": 3}],
        })

    monkeypatch.setattr(steam_auth, "_urlopen", fake_urlopen)
    session = steam_auth.start_qr_session("test-device")

    assert calls["method"] == "POST"
    assert calls["url"] == steam_auth.BEGIN_QR_URL
    assert calls["body"]["device_details"]["platform_type"] == 2
    assert calls["body"]["device_details"]["device_friendly_name"] == "test-device"
    assert session.client_id == "12345"
    assert session.request_id == "req-1"
    assert session.interval == 5
    assert session.challenge_url == "https://s.team/q/1/12345"
    assert session.code_types == [3]
    assert session.requires_code
    assert session.needs_approval


def test_start_qr_session_http_error(monkeypatch):
    def fake_urlopen(req, timeout=0):
        raise _http_error(500, "boom")

    monkeypatch.setattr(steam_auth, "_urlopen", fake_urlopen)
    with pytest.raises(SteamAuthError) as exc:
        steam_auth.start_qr_session()
    assert exc.value.status == 500


def _http_error(code, text):
    import urllib.error
    return urllib.error.HTTPError(steam_auth.BEGIN_QR_URL, code, "err", [], FakeResp(text))


# --------------------------------------------------------------------------
# Poll
# --------------------------------------------------------------------------


def test_poll_session_authorized(monkeypatch):
    def fake_urlopen(req, timeout=0):
        body = dict(urllib.parse.parse_qsl(req.data.decode()))
        assert body["client_id"] == "12345"
        assert body["request_id"] == "req-1"
        return _json_resp({
            "had_remote_interaction": True,
            "refresh_token": "rt",
            "access_token": "at",
            "account_name": "alice",
            "new_client_id": "99999",
        })

    monkeypatch.setattr(steam_auth, "_urlopen", fake_urlopen)
    result = steam_auth.poll_session("12345", "req-1")
    assert result.authorized
    assert result.refresh_token == "rt"
    assert result.access_token == "at"
    assert result.account_name == "alice"
    assert result.new_client_id == "99999"


def test_poll_session_not_interacted(monkeypatch):
    monkeypatch.setattr(
        steam_auth, "_urlopen", lambda req, timeout=0: _json_resp({"had_remote_interaction": False})
    )
    result = steam_auth.poll_session("12345", "req-1")
    assert not result.authorized
    assert not result.had_remote_interaction


def test_poll_session_expired(monkeypatch):
    def fake_urlopen(req, timeout=0):
        raise _http_error(404, "expired")

    monkeypatch.setattr(steam_auth, "_urlopen", fake_urlopen)
    result = steam_auth.poll_session("12345", "req-1")
    assert result.expired


def test_update_session_with_guard_code(monkeypatch):
    calls = {}

    def fake_urlopen(req, timeout=0):
        calls["body"] = dict(urllib.parse.parse_qsl(req.data.decode()))
        return _json_resp({})

    monkeypatch.setattr(steam_auth, "_urlopen", fake_urlopen)
    steam_auth.update_session_with_guard_code("12345", "ABCDE", 3, steamid="7656")
    assert calls["body"]["client_id"] == "12345"
    assert calls["body"]["code"] == "ABCDE"
    assert calls["body"]["code_type"] == "3"
    assert calls["body"]["steamid"] == "7656"


# --------------------------------------------------------------------------
# RSA password encryption
# --------------------------------------------------------------------------

# Mersenne primes so we can build a real RSA key without a crypto library.
_P = 2**127 - 1
_Q = 2**89 - 1
_N = _P * _Q
_E = 65537
_D = pow(_E, -1, (_P - 1) * (_Q - 1))
_SIZE = (_N.bit_length() + 7) // 8


def test_encrypt_password_roundtrip():
    encrypted = steam_auth._encrypt_password("hunter2", _N, _E)
    assert len(encrypted) == _SIZE
    decrypted = pow(int.from_bytes(encrypted, "big"), _D, _N).to_bytes(_SIZE, "big")
    assert decrypted[0:2] == b"\x00\x02"
    assert b"\x00" + b"hunter2" in decrypted


def test_encrypt_password_random_padding():
    a = steam_auth._encrypt_password("hunter2", _N, _E)
    b = steam_auth._encrypt_password("hunter2", _N, _E)
    assert a != b


def test_encrypt_password_too_long():
    with pytest.raises(SteamAuthError):
        steam_auth._encrypt_password("x" * (_SIZE + 20), _N, _E)


# --------------------------------------------------------------------------
# Credentials
# --------------------------------------------------------------------------


def test_start_credentials_session(monkeypatch):
    calls = {}

    def fake_urlopen(req, timeout=0):
        if req.method == "GET":
            calls["rsa_url"] = req.full_url
            return _json_resp({
                "publickey_mod": hex(_N)[2:],
                "publickey_exp": hex(_E)[2:],
                "timestamp": 1234567,
            })
        calls["body"] = dict(urllib.parse.parse_qsl(req.data.decode()))
        return _json_resp({
            "client_id": "555",
            "request_id": "rr",
            "interval": 5,
            "steamid": "76561198123456789",
            "allowed_confirmations": [{"confirmation_type": 2}],
        })

    monkeypatch.setattr(steam_auth, "_urlopen", fake_urlopen)
    session = steam_auth.start_credentials_session("alice", "hunter2")

    assert steam_auth.RSA_KEY_URL in calls["rsa_url"]
    assert "account_name=alice" in calls["rsa_url"]
    body = calls["body"]
    assert body["account_name"] == "alice"
    assert body["encryption_timestamp"] == "1234567"
    assert body["encrypted_password"]
    assert body["persistence"] == "1"
    assert session.client_id == "555"
    assert session.steamid == "76561198123456789"
    assert session.code_types == [2]


def test_start_credentials_session_invalid(monkeypatch):
    def fake_urlopen(req, timeout=0):
        if req.method == "GET":
            return _json_resp({
                "publickey_mod": hex(_N)[2:],
                "publickey_exp": hex(_E)[2:],
                "timestamp": 1234567,
            })
        return _json_resp({})

    monkeypatch.setattr(steam_auth, "_urlopen", fake_urlopen)
    with pytest.raises(SteamAuthError) as exc:
        steam_auth.start_credentials_session("alice", "wrong")
    assert "Invalid account name or password" in str(exc.value)


def test_start_credentials_session_captcha(monkeypatch):
    def fake_urlopen(req, timeout=0):
        if req.method == "GET":
            return _json_resp({
                "publickey_mod": hex(_N)[2:],
                "publickey_exp": hex(_E)[2:],
                "timestamp": 1234567,
            })
        return _json_resp({"captcha_needed": True})

    monkeypatch.setattr(steam_auth, "_urlopen", fake_urlopen)
    with pytest.raises(SteamAuthError) as exc:
        steam_auth.start_credentials_session("alice", "wrong")
    assert "CAPTCHA" in str(exc.value)


# --------------------------------------------------------------------------
# finalizelogin -> steamLoginSecure
# --------------------------------------------------------------------------


def test_finalize_login(monkeypatch):
    transfer = {"url": "https://steamcommunity.com/login/settoken", "params": {"nonce": "n1", "auth": "a1"}}

    def fake_open(req, jar, timeout=0):
        url = req.full_url
        if url == steam_auth.COMMUNITY_URL:
            _make_cookie(jar, "sessionid", "session123")
            return FakeResp("", 200)
        if url == steam_auth.FINALIZE_URL:
            assert b"nonce=rt" in req.data
            assert b"sessionid=session123" in req.data
            return FakeResp(json.dumps({"steamID": "7656", "transfer_info": [transfer]}), 200)
        assert url == transfer["url"]
        body = dict(urllib.parse.parse_qsl(req.data.decode()))
        assert body["nonce"] == "n1"
        assert body["auth"] == "a1"
        assert body["steamID"] == "7656"
        _make_cookie(jar, "steamLoginSecure", "7656||h1:deadbeef")
        return FakeResp("", 200)

    monkeypatch.setattr(steam_auth, "_open_with_cookies", fake_open)
    cookie = steam_auth.finalize_login("rt")
    assert cookie == "7656||h1:deadbeef"


def test_finalize_login_rejected(monkeypatch):
    def fake_open(req, jar, timeout=0):
        if req.full_url == steam_auth.COMMUNITY_URL:
            _make_cookie(jar, "sessionid", "session123")
            return FakeResp("", 200)
        return FakeResp(json.dumps({"success": False, "error": 8}), 200)

    monkeypatch.setattr(steam_auth, "_open_with_cookies", fake_open)
    with pytest.raises(SteamAuthError):
        steam_auth.finalize_login("bad-token")


def test_finalize_login_missing_cookie(monkeypatch):
    transfer = {"url": "https://steamcommunity.com/login/settoken", "params": {"nonce": "n1", "auth": "a1"}}

    def fake_open(req, jar, timeout=0):
        if req.full_url == steam_auth.COMMUNITY_URL:
            _make_cookie(jar, "sessionid", "session123")
            return FakeResp("", 200)
        if req.full_url == steam_auth.FINALIZE_URL:
            return FakeResp(json.dumps({"steamID": "7656", "transfer_info": [transfer]}), 200)
        return FakeResp("", 200)  # no steamLoginSecure cookie

    monkeypatch.setattr(steam_auth, "_open_with_cookies", fake_open)
    with pytest.raises(SteamAuthError) as exc:
        steam_auth.finalize_login("rt")
    assert "login cookie" in str(exc.value)


# --------------------------------------------------------------------------
# JWT steamid
# --------------------------------------------------------------------------


def test_steamid_from_refresh_token():
    # header.payload.signature
    payload = "eyJzdWIiOiI3NjU2MTE5ODEyMzQ1Njc4OSJ9"
    token = f"eyJhbGciOiJIUzI1NiJ9.{payload}.sig"
    assert steam_auth.steamid_from_refresh_token(token) == "76561198123456789"


def test_steamid_from_refresh_token_bad():
    assert steam_auth.steamid_from_refresh_token("not-a-jwt") is None
    assert steam_auth.steamid_from_refresh_token("a.") is None
