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

_STEAM_RECOLOR_CSS = """
:root {
    --st-accent-1: 102, 108, 255;
    --st-accent-2: 135, 140, 255;
    --st-color-1: 17, 17, 17;
    --st-color-2: 30, 30, 30;
    --st-color-3: 20, 20, 20;
    --st-color-4: 24, 24, 24;
    --st-color-5: 38, 41, 44;
    --st-color-6: 38, 38, 41;
    --st-background: 10, 10, 10;
    --st-red: 240, 74, 74;
    --st-green: 36, 166, 90;
    --st-blue: 75, 137, 239;
    --st-blue-hover: 100, 154, 242;
    --st-yellow: 239, 141, 75;
    --st-border-radius: 8px;
    --st-store-max-width: 940px;
}
body {
    background-color: rgb(var(--st-color-1)) !important;
    color: #fff !important;
}
body .page_content_ctn,
body .page_content,
body .maincontent,
body ._22xtsolKcQit92o-LBeRWD {
    max-width: var(--st-store-max-width) !important;
    margin: 0 auto !important;
    background: unset !important;
}
body a { color: rgb(var(--st-blue)) !important; }
body a:hover { color: rgb(var(--st-blue-hover)) !important; }
.game_review_summary { font-weight: 600; color: rgb(var(--st-red)); }
.game_review_summary.mixed { color: rgb(var(--st-yellow)); }
.game_review_summary.positive { color: rgb(var(--st-green)); }
.game_review_summary.no_reviews,
.game_review_summary.not_enough_reviews { color: #929396; }
.discount_block { display: flex !important; align-items: center !important; gap: 6px !important; padding: 6px !important; border-radius: 8px !important; color: #fff !important; background-color: rgb(var(--st-color-1)) !important; }
.discount_block .discount_pct { padding: 4px 6px !important; border-radius: 8px !important; color: rgb(var(--st-green)) !important; background-color: rgb(var(--st-green), .3) !important; }
.discount_block .discount_prices { display: flex !important; flex-direction: column !important; padding: 0 !important; background-color: unset !important; }
.discount_block .discount_prices .discount_final_price { color: rgb(var(--st-green)) !important; }
.discount_block.no_discount .discount_prices .discount_final_price { color: #fff !important; }
body::-webkit-scrollbar { background-color: rgb(var(--st-color-1)) !important; }
body::-webkit-scrollbar-thumb { border-radius: 8px !important; border: 5px solid transparent !important; background-clip: content-box !important; background-color: rgb(var(--st-accent-1)) !important; }
body::-webkit-scrollbar-thumb:hover { border: 4px solid transparent !important; background-color: rgb(var(--st-accent-2)) !important; }
"""

_STEAM_RECOLOR_JS = (
    "var s=document.createElement('style');"
    "s.textContent=%s;"
    "document.head.appendChild(s);"
) % repr(_STEAM_RECOLOR_CSS)


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
        self._view.setStyleSheet("background-color: #111111;")
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
