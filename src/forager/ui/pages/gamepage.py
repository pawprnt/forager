from __future__ import annotations
from PySide6.QtCore import Qt, Signal
from PySide6.QtGui import QPixmap, QFont, QColor
from PySide6.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QLabel, QPushButton, QScrollArea,
    QFrame, QListWidget, QListWidgetItem,
)

from forager.core.game import Game, Source
from forager.artwork import pipeline as art
from forager.providers.steam import credentials
from forager.providers.steam.achievements import (
    achievement_summary,
    player_achievements,
)
from forager.ui.fonts import UI_FONT
from forager.ui.widgets.banner import Banner
from forager.ui.theme import C
from forager.ui.icons import load_icon as load_bundled_icon
from forager.ui import style

_PLAY_QSS = style.button_qss("play")
_RUNNING_QSS = style.button_qss("running")


class GamePage(QWidget):
    play = Signal(object)
    stop = Signal(object)
    install = Signal(object)
    back_requested = Signal()

    def __init__(self, parent=None):
        super().__init__(parent)
        self.game: Game | None = None
        self._logo: QPixmap | None = None
        self._running = False

        self.setStyleSheet(style.surface(1))

        outer = QVBoxLayout(self)
        outer.setContentsMargins(16, 16, 16, 16)
        outer.setSpacing(0)

        scroll = QScrollArea()
        scroll.setWidgetResizable(True)
        scroll.setFrameShape(QFrame.Shape.NoFrame)
        scroll.setStyleSheet(f"QScrollArea {{ {style.surface(1)} border: none; }}")

        content = QWidget()
        content.setStyleSheet(style.surface(1))
        v = QVBoxLayout(content)
        v.setContentsMargins(0, 0, 0, 16)
        v.setSpacing(16)

        self._banner = Banner()
        v.addWidget(self._banner)

        self._title = QLabel("")
        self._title.setFont(QFont(UI_FONT, 26, QFont.Weight.Bold))
        self._title.setStyleSheet(style.label(self._title, C.TEXT))
        self._title.setWordWrap(True)
        v.addWidget(self._title)

        info_row = QHBoxLayout()
        info_row.setSpacing(12)

        self._play_btn = QPushButton()
        self._play_btn.setCursor(Qt.CursorShape.PointingHandCursor)
        self._play_btn.setFixedHeight(48)
        self._play_btn.setMinimumWidth(220)
        self._play_btn.setStyleSheet(_PLAY_QSS)
        play_lay = QHBoxLayout(self._play_btn)
        play_lay.setContentsMargins(20, 0, 16, 0)
        play_lay.setSpacing(5)
        self._play_icon_label = QLabel()
        self._play_icon_label.setPixmap(load_bundled_icon("play", "#ffffff").pixmap(20, 20))
        self._play_icon_label.setStyleSheet("background: transparent;")
        play_lay.addWidget(self._play_icon_label)
        self._play_text = QLabel("Play")
        self._play_text.setStyleSheet(style.label(self._play_text, "#ffffff", size=17, weight=600))
        play_lay.addWidget(self._play_text)
        play_lay.addStretch(1)
        self._play_btn.clicked.connect(self._on_play)
        info_row.addWidget(self._play_btn)

        self._source_badge = QLabel("")
        self._source_badge.setStyleSheet(
            f"color: {C.TEXT_DIM}; background-color: {C.COLOR_3}; "
            f"font-size: 12px; padding: 6px 12px; border-radius: {C.RADIUS}px;"
        )
        info_row.addWidget(self._source_badge, alignment=Qt.AlignmentFlag.AlignVCenter)

        self._path_label = QLabel("")
        self._path_label.setStyleSheet(style.label(self._path_label, C.TEXT_DIM, size=12))
        self._path_label.setWordWrap(True)
        info_row.addWidget(self._path_label, stretch=1)

        v.addLayout(info_row)

        self._info_box = self._build_info_box()
        v.addWidget(self._info_box, alignment=Qt.AlignmentFlag.AlignLeft)

        self._ach_frame = self._build_achievements_box()
        self._ach_frame.hide()
        v.addWidget(self._ach_frame)

        v.addStretch(1)

        scroll.setWidget(content)
        outer.addWidget(scroll)

        self._banner.set_overlay(self._build_banner_overlay())
        self._banner_overlay = self._banner._overlay

    def _build_banner_overlay(self) -> QWidget:
        overlay = QWidget(self._banner)
        overlay.setStyleSheet("background: transparent;")

        lay = QVBoxLayout(overlay)
        lay.setContentsMargins(20, 16, 20, 20)
        lay.setSpacing(12)

        top = QHBoxLayout()
        back_btn = QPushButton("‹  Library")
        back_btn.setCursor(Qt.CursorShape.PointingHandCursor)
        back_btn.setStyleSheet(
            f"""
            QPushButton {{
                background-color: rgba(17, 17, 17, 170); color: {C.TEXT};
                border: none; border-radius: {C.RADIUS}px; padding: 8px 14px;
                font-size: 13px; font-weight: 600;
            }}
            QPushButton:hover {{ background-color: rgba(17, 17, 17, 230); }}
            """
        )
        back_btn.clicked.connect(self.back_requested)
        top.addWidget(back_btn)
        top.addStretch(1)
        lay.addLayout(top)

        lay.addStretch(1)

        self._logo_label = QLabel()
        self._logo_label.setAlignment(Qt.AlignmentFlag.AlignBottom | Qt.AlignmentFlag.AlignLeft)
        self._logo_label.setMaximumWidth(520)
        self._logo_label.setMinimumHeight(90)
        lay.addWidget(self._logo_label)

        return overlay

    def _build_info_box(self) -> QFrame:
        box = QFrame()
        box.setFixedWidth(300)
        box.setStyleSheet(style.panel(box, 2))
        v = QVBoxLayout(box)
        v.setContentsMargins(14, 12, 14, 12)
        v.setSpacing(10)

        header = QLabel("GAME INFO")
        header.setStyleSheet(style.label(header, C.TEXT_DIM, size=11, weight=700, letter_spacing=1))
        v.addWidget(header)

        self._info_rows: dict[str, QLabel] = {}
        for key, label in (("Source", "source"), ("App ID", "app_id")):
            row = QHBoxLayout()
            row.setSpacing(8)
            k = QLabel(key.upper())
            k.setStyleSheet(style.label(k, C.TEXT_DIM, size=11))
            k.setFixedWidth(70)
            val = QLabel("")
            val.setStyleSheet(style.label(val, C.TEXT, size=12))
            val.setWordWrap(True)
            row.addWidget(k)
            row.addWidget(val, stretch=1)
            v.addLayout(row)
            self._info_rows[label] = val

        v.addStretch(1)
        return box

    def _on_play(self):
        if self.game is None:
            return
        if not self.game.installed or self.game.path is None:
            self.install.emit(self.game)
            return
        if self._running:
            self.stop.emit(self.game)
        else:
            self.play.emit(self.game)

    def _build_achievements_box(self) -> QFrame:
        box = QFrame()
        box.setStyleSheet(style.panel(box, 2))
        v = QVBoxLayout(box)
        v.setContentsMargins(14, 12, 14, 12)
        v.setSpacing(10)

        self._ach_header = QLabel("ACHIEVEMENTS")
        self._ach_header.setStyleSheet(
            style.label(self._ach_header, C.TEXT_DIM, size=11, weight=700, letter_spacing=1)
        )
        v.addWidget(self._ach_header)

        self._ach_list = QListWidget()
        self._ach_list.setStyleSheet(
            "QListWidget { background: transparent; border: none; }"
        )
        self._ach_list.setMaximumHeight(180)
        v.addWidget(self._ach_list)
        return box

    def _populate_achievements(self, game: Game) -> None:
        steamid = credentials.get_steamid()
        if game.source != Source.STEAM or not game.app_id or not steamid:
            self._ach_frame.hide()
            return
        achievements = player_achievements(steamid, game.app_id)
        self._ach_list.clear()
        if not achievements:
            self._ach_frame.hide()
            return
        earned, total, _frac = achievement_summary(achievements)
        self._ach_header.setText(f"ACHIEVEMENTS  ({earned}/{total})")
        for ach in achievements:
            mark = "✔" if ach["achieved"] else "○"
            item = QListWidgetItem(f"{mark}  {ach['name']}")
            item.setForeground(QColor(C.ACCENT if ach["achieved"] else C.TEXT_DIM))
            self._ach_list.addItem(item)
        self._ach_frame.show()

    def set_game(self, game: Game):
        self.game = game
        self._running = False

        hero = art.load_hero(game, allow_network=False)
        if hero is None:
            hero = art.placeholder_card(game, 900, 420)
            self._banner.set_source(hero, fit=True)
        else:
            self._banner.set_source(hero)

        self._logo = art.load_logo(game)
        if self._logo is not None:
            self._logo_label.setPixmap(
                self._logo.scaledToWidth(
                    min(520, self._logo.width()),
                    Qt.TransformationMode.SmoothTransformation,
                )
            )
            self._title.setText("")
        else:
            self._logo_label.clear()
            self._title.setText(game.name.replace("/", " / "))

        self._source_badge.setText(game.source_name)
        self._path_label.setText(game.display_path)
        self._info_rows["source"].setText(game.source_name)
        self._info_rows["app_id"].setText(game.app_id or "—")
        self._populate_achievements(game)

        installed = game.installed and game.path is not None
        if installed:
            self._play_btn.setStyleSheet(_PLAY_QSS)
            self._play_icon_label.setPixmap(load_bundled_icon("play", "#ffffff").pixmap(20, 20))
            self._play_text.setText("Play" if not self._running else "Stop")
        else:
            self._play_btn.setStyleSheet(_PLAY_QSS)
            self._play_icon_label.setPixmap(load_bundled_icon("box", "#ffffff").pixmap(20, 20))
            self._play_text.setText("Install")

    def set_running(self, running: bool) -> None:
        if self.game is None:
            return
        if not (self.game.installed and self.game.path is not None):
            return
        if running == self._running:
            return
        self._running = running
        if running:
            self._play_btn.setStyleSheet(_RUNNING_QSS)
            self._play_icon_label.setPixmap(load_bundled_icon("stop", "#ffffff").pixmap(20, 20))
            self._play_text.setText("Stop")
        else:
            self._play_btn.setStyleSheet(_PLAY_QSS)
            self._play_icon_label.setPixmap(load_bundled_icon("play", "#ffffff").pixmap(20, 20))
            self._play_text.setText("Play")

    def set_hero(self, pix: QPixmap | None):
        if pix is None:
            return
        self._banner.set_source(pix)
