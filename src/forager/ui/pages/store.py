"""Store page (roadmap item 4).

A SpaceTheme-style top tab switcher with one tab per storefront.  The Steam
tab hosts an embedded Chromium webview (``PySide6-WebEngine``) of the real
store, recolored with an injected stylesheet to match SpaceTheme.  The other
storefronts remain placeholder panes until their backends land.
"""
from __future__ import annotations

from PySide6.QtCore import Qt
from PySide6.QtGui import QFont
from PySide6.QtWidgets import (
    QWidget, QLabel, QPushButton, QButtonGroup, QStackedWidget,
    QVBoxLayout, QHBoxLayout, QFrame,
)

from forager.ui.fonts import UI_FONT
from forager.ui import style
from forager.ui.theme import C, TAB_QSS, PAGE_BG

try:
    from PySide6.QtWebEngineWidgets import QWebEngineView
except ImportError:
    QWebEngineView = None

_STORES = ("Steam", "Epic Games", "GOG", "itch.io")

_STEAM_URL = "https://store.steampowered.com/"

_STEAM_RECOLOR_JS = (
    "var s=document.createElement('style');"
    "s.textContent='html,body{background-color:%s !important;}';"
    "document.head.appendChild(s);"
) % PAGE_BG


class WebStorePane(QWidget):
    """Embedded store webview with a SpaceTheme background recolor."""

    def __init__(self, url: str, parent=None):
        super().__init__(parent)
        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        if QWebEngineView is None:
            note = QLabel("PySide6-WebEngine is not installed.\nInstall it to browse the store in-app.")
            note.setAlignment(Qt.AlignmentFlag.AlignCenter)
            style.label(note, C.TEXT_DIM, size=14)
            layout.addWidget(note)
            self._view = None
            return
        self._view = QWebEngineView()
        self._view.setStyleSheet(f"background-color: {PAGE_BG};")
        self._view.loadFinished.connect(self._on_load_finished)
        layout.addWidget(self._view)
        self._url = url

    def load(self):
        if self._view is not None:
            self._view.load(self._url)

    def _on_load_finished(self, ok: bool):
        if self._view is not None and ok:
            self._view.page().runJavaScript(_STEAM_RECOLOR_JS)


class StorePage(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setStyleSheet(style.bg())
        layout = QVBoxLayout(self)
        layout.setContentsMargins(24, 18, 24, 18)
        layout.setSpacing(12)

        header = QLabel("Store")
        header.setFont(QFont(UI_FONT, 22, QFont.Weight.Bold))
        style.label(header, C.TEXT)
        layout.addWidget(header)

        self._stack = QStackedWidget()
        self._web_pane = WebStorePane(_STEAM_URL)
        self._stack.addWidget(self._web_pane)
        for name in _STORES[1:]:
            self._stack.addWidget(self._placeholder(name))
        layout.addWidget(self._build_tabs())
        layout.addWidget(self._stack, stretch=1)

        self._web_pane.load()

    def _build_tabs(self) -> QWidget:
        bar = QWidget()
        bar.setStyleSheet(style.surface_qss(2))
        bar_layout = QHBoxLayout(bar)
        bar_layout.setContentsMargins(6, 6, 6, 6)
        bar_layout.setSpacing(6)

        group = QButtonGroup(self)
        group.setExclusive(True)
        for index, name in enumerate(_STORES):
            btn = QPushButton(name)
            btn.setCheckable(True)
            btn.setCursor(Qt.CursorShape.PointingHandCursor)
            btn.setStyleSheet(TAB_QSS)
            group.addButton(btn, index)
            bar_layout.addWidget(btn)
        self._tabs_group = group
        self._tabs_group.buttonClicked.connect(self._switch_tab)
        group.button(0).setChecked(True)
        return bar

    def _switch_tab(self, button: QPushButton):
        index = self._tabs_group.id(button)
        if index < 0:
            return
        self._stack.setCurrentIndex(index)
        if index == 0:
            self._web_pane.load()

    def _placeholder(self, name: str) -> QWidget:
        page = QFrame()
        style.panel(page, 2)
        v = QVBoxLayout(page)
        v.addStretch(1)
        store = QLabel(name)
        store.setAlignment(Qt.AlignmentFlag.AlignCenter)
        style.label(store, C.TEXT_DIM, size=22, weight=700)
        note = QLabel("Store integration coming soon")
        note.setAlignment(Qt.AlignmentFlag.AlignCenter)
        style.label(note, C.TEXT_DIM, size=13)
        v.addWidget(store)
        v.addWidget(note)
        v.addStretch(1)
        return page
