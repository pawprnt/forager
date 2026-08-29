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
