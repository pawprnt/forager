import sys

import pytest

pytest.importorskip("PySide6")

from PySide6.QtWidgets import QApplication

from forager.ui.pages import store as store_page


@pytest.fixture(scope="module")
def app():
    return QApplication.instance() or QApplication(sys.argv)


def test_recolor_js_loads_css():
    assert "spacetheme-css" in store_page._STEAM_RECOLOR_JS
    assert "createElement" in store_page._STEAM_RECOLOR_JS


def test_store_page_constructs(app):
    page = store_page.StorePage()
    assert page._stack.count() == 4
    if store_page.QWebEngineView is None:
        assert page._web_pane._view is None
    page.deleteLater()


def test_steam_tab_switches(app):
    page = store_page.StorePage()
    page._switch_tab(page._tabs_group.button(0))
    assert page._stack.currentIndex() == 0
    page.deleteLater()
