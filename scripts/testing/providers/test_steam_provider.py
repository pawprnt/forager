from pathlib import Path

from forager.providers.steam import account as steam
from forager.providers.steam import credentials as cred
from forager.providers.steam import depotdownloader as dd


class FakeKeyring:
    def __init__(self):
        self._store = {}

    def set_password(self, service, user, password):
        self._store[(service, user)] = password

    def get_password(self, service, user):
        return self._store.get((service, user))

    def delete_password(self, service, user):
        self._store.pop((service, user), None)


def test_credentials_roundtrip(monkeypatch):
    keyring = FakeKeyring()
    monkeypatch.setattr(cred, "_keyring", keyring)
    assert not steam.has_credentials()
    steam.set_credentials("alice", "hunter2")
    assert steam.get_username() == "alice"
    assert steam.get_password() == "hunter2"
    assert steam.has_credentials()
    steam.clear_credentials()
    assert steam.get_username() is None
    assert steam.get_password() is None
    assert not steam.has_credentials()


def test_credentials_no_keyring(monkeypatch):
    monkeypatch.setattr(cred, "_keyring", None)
    assert steam.get_username() is None
    assert steam.get_password() is None
    assert not steam.has_credentials()


def test_login_cmd(monkeypatch):
    monkeypatch.setattr(dd, "depotdownloader_bin", lambda: Path("/bin/depotdownloader"))
    cmd = dd._login_cmd("alice", "hunter2", False, Path("/tmp/dl"))
    assert cmd[0] == "/bin/depotdownloader"
    assert "-app" in cmd and "-depot" in cmd and "-manifest-only" in cmd
    assert cmd[cmd.index("-username") + 1] == "alice"
    assert cmd[cmd.index("-password") + 1] == "hunter2"
    assert "-remember-password" not in cmd
    cmd2 = dd._login_cmd("alice", "hunter2", True, Path("/tmp/dl"))
    assert "-remember-password" in cmd2


def test_session_cmd(monkeypatch):
    monkeypatch.setattr(dd, "depotdownloader_bin", lambda: Path("/bin/depotdownloader"))
    cmd = dd._session_cmd("alice", Path("/tmp/dl"))
    assert cmd[cmd.index("-username") + 1] == "alice"
    assert "-remember-password" in cmd
    assert "-password" not in cmd and "-qr" not in cmd


def test_login_method_roundtrip(monkeypatch):
    keyring = FakeKeyring()
    monkeypatch.setattr(cred, "_keyring", keyring)
    assert steam.get_login_method() is None
    steam.set_credentials("alice", "hunter2")
    assert steam.get_login_method() == "password"
    steam.set_web_username("alice")
    assert steam.get_username() == "alice"
    assert steam.get_password() is None
    assert steam.has_credentials()
    assert steam.get_login_method() == "web"
    steam.clear_credentials()
    assert steam.get_username() is None
    assert steam.get_login_method() is None
    assert not steam.has_credentials()


def test_login_method_infers_password_for_legacy(monkeypatch):
    keyring = FakeKeyring()
    monkeypatch.setattr(cred, "_keyring", keyring)
    keyring.set_password("forager", "steam_username", "alice")
    keyring.set_password("forager", "steam_password", "hunter2")
    assert steam.get_login_method() == "password"


def test_set_web_username_clears_password(monkeypatch):
    keyring = FakeKeyring()
    monkeypatch.setattr(cred, "_keyring", keyring)
    steam.set_credentials("alice", "hunter2")
    steam.set_web_username("alice")
    assert keyring._store[("forager", "steam_login_method")] == "web"
    assert ("forager", "steam_password") not in keyring._store


def test_set_steam_session_qr(monkeypatch):
    keyring = FakeKeyring()
    monkeypatch.setattr(cred, "_keyring", keyring)
    steam.set_steam_session(
        "alice", method="qr",
        steamid="76561198123456789", login_secure="76561198123456789||h1:deadbeef",
    )
    assert steam.get_username() == "alice"
    assert steam.get_login_method() == "qr"
    assert steam.get_password() is None
    assert steam.get_steamid() == "76561198123456789"
    assert steam.get_login_secure() == "76561198123456789||h1:deadbeef"


def test_set_steam_session_password(monkeypatch):
    keyring = FakeKeyring()
    monkeypatch.setattr(cred, "_keyring", keyring)
    steam.set_steam_session(
        "alice", method="password", password="hunter2",
        steamid="76561198123456789", login_secure="76561198123456789||h1:deadbeef",
    )
    assert steam.get_login_method() == "password"
    assert steam.get_password() == "hunter2"
    assert steam.get_steamid() == "76561198123456789"
    assert steam.get_login_secure() == "76561198123456789||h1:deadbeef"
    steam.clear_credentials()
    assert steam.get_username() is None
    assert steam.get_steamid() is None
    assert steam.get_login_secure() is None
    assert steam.get_password() is None


def test_set_steam_session_clears_password_for_qr(monkeypatch):
    keyring = FakeKeyring()
    monkeypatch.setattr(cred, "_keyring", keyring)
    steam.set_credentials("alice", "hunter2")
    steam.set_steam_session("alice", method="qr", steamid="76561198123456789")
    assert steam.get_password() is None
    assert ("forager", "steam_password") not in keyring._store


def test_steamid_from_cookie():
    assert steam.steamid_from_cookie("76561198123456789||h1:deadbeef") == "76561198123456789"
    assert steam.steamid_from_cookie("76561198123456789") == "76561198123456789"
    assert steam.steamid_from_cookie("") is None
    assert steam.steamid_from_cookie("||foo") is None
    assert steam.steamid_from_cookie("abc||x") is None


def test_account_name_from_steamid(monkeypatch):
    import urllib.request

    xml = (
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<profile>\n"
        "  <steamID64>76561198123456789</steamID64>\n"
        "  <steamID>alice &amp; friends</steamID>\n"
        "  <onlineState>0</onlineState>\n"
        "</profile>\n"
    )

    class FakeResp:
        def read(self, _n):
            return xml.encode("utf-8")

        def __enter__(self):
            return self

        def __exit__(self, *a):
            return False

    def fake_urlopen(req, timeout=0):
        assert b"/profiles/76561198123456789/" in req.full_url.encode()
        return FakeResp()

    monkeypatch.setattr(urllib.request, "urlopen", fake_urlopen)
    assert steam.account_name_from_steamid("76561198123456789") == "alice & friends"


def test_account_name_from_steamid_handles_errors(monkeypatch):
    import urllib.request

    def boom(req, timeout=0):
        raise OSError("network down")

    monkeypatch.setattr(urllib.request, "urlopen", boom)
    assert steam.account_name_from_steamid("76561198123456789") is None


def test_verify_session_reuses_token(monkeypatch):
    calls = {}

    def fake_run_dd(cmd, timeout, cancel_event=None, on_line=None):
        calls["cmd"] = cmd
        return ["Connecting to Steam3...", " Done!", "Got 3 licenses for account!"], "", 0, False

    monkeypatch.setattr(dd, "_run_dd", fake_run_dd)
    ok, detail = steam.verify_session("alice")
    assert ok and detail == "Signed in as alice"
    assert calls["cmd"][calls["cmd"].index("-username") + 1] == "alice"
    assert "-password" not in calls["cmd"]


def test_verify_session_rejected_token(monkeypatch):
    monkeypatch.setattr(
        dd, "_run_dd",
        lambda cmd, timeout, cancel_event=None, on_line=None: (
            ["Connecting to Steam3...", "Done!", "Access token was rejected (Expired).",
             "Unable to get steam3 credentials."], "", 1, False,
        ),
    )
    ok, detail = steam.verify_session("alice")
    assert not ok and "rejected" in detail


def test_verify_session_no_stored_token(monkeypatch):
    monkeypatch.setattr(
        dd, "_run_dd",
        lambda cmd, timeout, cancel_event=None, on_line=None: (
            [], 'Enter account password for "alice": ', 1, False,
        ),
    )
    ok, detail = steam.verify_session("alice")
    assert not ok and "sign in again" in detail
