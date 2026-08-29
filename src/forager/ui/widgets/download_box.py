"""Compact sidebar download box shown while a download is active."""
from __future__ import annotations
from PySide6.QtCore import Qt, Signal
from PySide6.QtWidgets import QFrame, QLabel, QVBoxLayout, QHBoxLayout

from forager.ui import style
from forager.ui.theme import C
from forager.ui.pages.downloads import ProgressBar, format_size


class DownloadBox(QFrame):
    """Compact sidebar card shown only while a download is active."""

    clicked = Signal()

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setCursor(Qt.CursorShape.PointingHandCursor)
        self.setObjectName("downloadBox")
        self.setStyleSheet(
            style.surface_qss(2)
            + f"#downloadBox:hover {{ background-color: {C.COLOR_3}; }}"
        )

        layout = QVBoxLayout(self)
        layout.setContentsMargins(12, 10, 12, 10)
        layout.setSpacing(6)

        top = QHBoxLayout()
        top.setSpacing(6)
        self._name = QLabel("Downloading")
        self._name.setStyleSheet(style.label(self._name, C.TEXT, size=12, weight=600))
        self._percent = QLabel("0%")
        self._percent.setStyleSheet(style.label(self._percent, C.ACCENT_2, size=12, weight=600))
        top.addWidget(self._name)
        top.addStretch(1)
        top.addWidget(self._percent)
        layout.addLayout(top)

        self._bar = ProgressBar(parent=self)
        layout.addWidget(self._bar)

        self._detail = QLabel("")
        self._detail.setStyleSheet(style.label(self._detail, "#b8bcbf", size=11))
        layout.addWidget(self._detail)

        self.hide()

    def mousePressEvent(self, event):
        self.clicked.emit()
        super().mousePressEvent(event)

    def begin(self, name: str) -> None:
        self._name.setText(name)
        self._percent.setText("0%")
        self._bar.set_value(0)
        self._detail.setText("")
        self.show()

    def set_progress(self, progress) -> None:
        percent = f"{progress.percent:.0f}%"
        self._percent.setText(percent)
        self._bar.set_value(progress.percent)
        if progress.stage.lower() == "downloading":
            bits = [f"{format_size(progress.done)} / {format_size(progress.total)}"]
            if progress.speed > 0:
                bits.append(f"{format_size(progress.speed)}/s")
            self._detail.setText(" \u00b7 ".join(bits))
        else:
            self._detail.setText(f"{progress.stage}\u2026")

    def hide_download(self) -> None:
        self.hide()
