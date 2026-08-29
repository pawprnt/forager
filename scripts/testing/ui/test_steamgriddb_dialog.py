import sys

import pytest

pytest.importorskip("PySide6")

from PySide6.QtWidgets import QApplication

from forager.ui.dialogs import steamgriddb_dialog as sgdb_dialog


@pytest.fixture(scope="module")
def app():
    return QApplication.instance() or QApplication(sys.argv)


def test_token_dialog_closes_without_webengine(monkeypatch, app):
    monkeypatch.setattr(sgdb_dialog, "QWebEngineView", None)
    dlg = sgdb_dialog.SteamGridDBTokenDialog()
    try:
        dlg.close()
    except AttributeError:
        pytest.fail("closeEvent referenced a missing _view when WebEngine is absent")
