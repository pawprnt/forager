import sys

import pytest

pytest.importorskip("PySide6")

from PySide6.QtWidgets import QApplication

from forager.ui.dialogs.settings import SettingsDialog


@pytest.fixture(scope="module")
def app():
    return QApplication.instance() or QApplication(sys.argv)


def test_settings_dialog_constructs(app):
    dlg = SettingsDialog()
    try:
        dlg.show()
        QApplication.processEvents()
        assert dlg._pages is not None
        assert dlg._account is not None
    finally:
        dlg.close()
        QApplication.processEvents()
