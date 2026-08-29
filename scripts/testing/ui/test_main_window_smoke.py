import sys

import pytest

pytest.importorskip("PySide6")

from PySide6.QtWidgets import QApplication

from forager.ui.main_window import MainWindow


@pytest.fixture(scope="module")
def app():
    return QApplication.instance() or QApplication(sys.argv)


def test_main_window_constructs(app):
    win = MainWindow()
    try:
        win.show()
        QApplication.processEvents()
        assert win._grid is not None
        assert win._sidebar is not None
        assert win._gamepage is not None
    finally:
        win.close()
        QApplication.processEvents()


def test_downloads_settings_button_opens_settings(app, monkeypatch):
    win = MainWindow()
    try:
        called = {}
        monkeypatch.setattr(win, "_open_settings", lambda: called.setdefault("opened", True))
        win._downloads_page.settings_requested.emit()
        assert called.get("opened")
    finally:
        win.close()
        QApplication.processEvents()
