"""Generated placeholder art for games with no cover art at all.

``placeholder_card`` renders the sunburst banner used on the game page;
``placeholder_grid`` renders the glow cover used for grid tiles. Both draw the
game's local icon (bare, no card) with a soft black shadow offset to the
bottom-right, then the game name in bundled VT323 type.
"""
from __future__ import annotations
import math
from pathlib import Path
from PySide6.QtCore import Qt, QPointF, QRectF
from PySide6.QtGui import (
    QPixmap, QImage, QPainter, QColor, QFont, QFontDatabase,
    QRadialGradient, QLinearGradient, QTextOption,
)

from forager.core.game import Game
from forager.core.paths import resources_dir
from forager.services.icon_provider import load_icon
from forager.ui.theme import C
from forager.artwork.pixmap_utils import scale_crop

_PLACEHOLDER_FONT_NAME = "VT323"
_FONT_FILE = resources_dir() / "fonts" / "VT323-Regular.ttf"
_PLACEHOLDER_FONT_FAMILY: str | None = None
_PLACEHOLDER_SHADOW_CACHE: dict[int, QImage] = {}


def register_placeholder_font() -> str:
    """Register the bundled VT323 font and return its family name.

    Called at app startup and lazily again before any placeholder renders, so
    it also works for tests and workers. Falls back to the font name when the
    font cannot be registered (e.g. no QApplication yet).
    """
    global _PLACEHOLDER_FONT_FAMILY
    if _PLACEHOLDER_FONT_FAMILY is None:
        family = None
        if _FONT_FILE.is_file():
            try:
                fid = QFontDatabase.addApplicationFont(str(_FONT_FILE))
                if fid != -1:
                    fams = QFontDatabase.applicationFontFamilies(fid)
                    if fams:
                        family = fams[0]
            except Exception:
                family = None
        _PLACEHOLDER_FONT_FAMILY = family or _PLACEHOLDER_FONT_NAME
    return _PLACEHOLDER_FONT_FAMILY


def placeholder_card(game: Game, width: int, height: int, name: str | None = None) -> QPixmap:
    """Banner placeholder: the wide fallback used on the game page."""
    return _render_placeholder(game, _paint_surface, width, height, name)


def placeholder_grid(game: Game, width: int, height: int, name: str | None = None) -> QPixmap:
    """Cover placeholder, rendered at 600x900 then cropped to the tile."""
    cover = _render_placeholder(game, _paint_surface, 600, 900, name, pts=125)
    return scale_crop(cover, width, height)


def _black_shadow(pix: QPixmap, blur_scale: int = 8, dx: int = 3, dy: int = 3,
                  alpha: int = 158) -> QImage:
    """Soft black silhouette of *pix* offset down-right (light from top-left).

    The buffer is sized so the blur never clips at the edge, keeping the
    shadow small without a hard cutoff on the bottom/right.
    """
    key = pix.cacheKey()
    cached = _PLACEHOLDER_SHADOW_CACHE.get(key)
    if cached is not None:
        return cached
    img = pix.toImage().convertToFormat(QImage.Format.Format_ARGB32)
    w, h = img.width(), img.height()
    mask = QImage(w, h, QImage.Format.Format_ARGB32)
    mask.fill(QColor(0, 0, 0, 255))
    mp = QPainter(mask)
    mp.setCompositionMode(QPainter.CompositionMode.CompositionMode_DestinationIn)
    mp.drawImage(0, 0, img)
    mp.end()
    small = mask.scaled(
        max(2, w // blur_scale), max(2, h // blur_scale),
        Qt.AspectRatioMode.KeepAspectRatio, Qt.TransformationMode.SmoothTransformation,
    )
    blurred = small.scaled(
        w, h, Qt.AspectRatioMode.IgnoreAspectRatio, Qt.TransformationMode.SmoothTransformation,
    )
    m = 12
    out = QImage(w + 2 * m, h + 2 * m, QImage.Format.Format_ARGB32)
    out.fill(Qt.GlobalColor.transparent)
    op = QPainter(out)
    op.setOpacity(alpha / 255.0)
    op.drawImage(m + dx, m + dy, blurred)
    op.end()
    _PLACEHOLDER_SHADOW_CACHE[key] = out
    return out


def _paint_surface(p: QPainter, w: int, h: int):
    """Layered SpaceTheme surface: a subtle top-lit shelf, not a flat fill."""
    g = QLinearGradient(0, 0, 0, h)
    g.setColorAt(0.0, QColor(C.COLOR_2))
    g.setColorAt(1.0, QColor(C.COLOR_1))
    p.fillRect(0, 0, w, h, g)
    sheen = QLinearGradient(0, 0, 0, int(h * 0.4))
    sheen.setColorAt(0.0, QColor(255, 255, 255, 14))
    sheen.setColorAt(1.0, QColor(255, 255, 255, 0))
    p.fillRect(0, 0, w, h, sheen)


def _draw_monogram(p: QPainter, text: str, w: int, h: int):
    """Big accent first-letter mark when no icon is available."""
    ch = next((c for c in text if c.isalnum()), "?").upper()
    font = QFont(register_placeholder_font())
    font.setPointSize(int(min(w, h) * 0.40))
    font.setWeight(QFont.Weight.Bold)
    font.setLetterSpacing(QFont.SpacingType.PercentageSpacing, 100)
    p.setFont(font)
    p.setPen(QColor(C.ACCENT_1))
    p.drawText(QRectF(0, 0, w, h), ch, QTextOption(Qt.AlignmentFlag.AlignCenter))


def _draw_title_bar(p: QPainter, w: int, h: int, bar_h: int, text: str, pts: int):
    """Solid bottom label strip with a divider, so the title reads as a tile."""
    top = h - bar_h
    p.fillRect(0, top, w, bar_h, QColor(8, 10, 14, 205))
    p.setPen(QColor(C.COLOR_3))
    p.drawLine(0, top, w, top)
    font = QFont(register_placeholder_font())
    font.setPointSize(pts)
    font.setWeight(QFont.Weight.Normal)
    font.setLetterSpacing(QFont.SpacingType.PercentageSpacing, 104)
    p.setFont(font)
    p.setPen(QColor("#cdd6e2"))
    opt = QTextOption(Qt.AlignmentFlag.AlignCenter)
    opt.setWrapMode(QTextOption.WrapMode.WrapAtWordBoundaryOrAnywhere)
    p.drawText(QRectF(int(w * 0.06), top, int(w * 0.88), bar_h), text, opt)


def _local_icon_pixmap(game: Game) -> QPixmap | None:
    """Raw local icon (folder icon / .minecraft/icon.png / embedded .exe icon)
    at full resolution, unlike the 48px-capped ``load_icon``."""
    for name in ("icon.png", "icon.ico", "icon.svg", "Icon.png", "Icon.ico"):
        candidate = game.path / name
        if candidate.is_file():
            pix = QPixmap(str(candidate))
            if not pix.isNull():
                return pix
    mc = game.path / ".minecraft/icon.png"
    if mc.is_file():
        pix = QPixmap(str(mc))
        if not pix.isNull():
            return pix
    return _exe_icon_pixmap(game.path)


def _exe_icon_pixmap(path: Path) -> QPixmap | None:
    from forager.artwork.pe_icons import best_icon, find_exe_with_icon

    exe = find_exe_with_icon(path)
    if exe is None:
        return None
    try:
        im = best_icon(exe)
    except Exception:
        return None
    if im is None:
        return None
    qimg = QImage(im.tobytes("raw", "RGBA"), im.width, im.height,
                  QImage.Format.Format_RGBA8888)
    if qimg.isNull():
        return None
    return QPixmap.fromImage(qimg)


def _placeholder_icon(game: Game) -> QPixmap | None:
    raw = _local_icon_pixmap(game)
    if raw is not None:
        return raw
    return load_icon(game, allow_network=False)


def _render_placeholder(game: Game, background, width: int, height: int,
                        name: str | None = None, pts: int = 46) -> QPixmap:
    pix = QPixmap(width, height)
    pix.fill(Qt.GlobalColor.transparent)
    p = QPainter(pix)
    p.setRenderHint(QPainter.RenderHint.SmoothPixmapTransform)
    background(p, width, height)
    bar_h = max(40, int(height * 0.22))
    content_h = height - bar_h
    icon = _placeholder_icon(game)
    if icon is not None:
        side = int(min(width, content_h) * 0.42)
        scaled = icon.scaled(side, side, Qt.AspectRatioMode.KeepAspectRatio,
                             Qt.TransformationMode.SmoothTransformation)
        x = (width - scaled.width()) // 2
        y = int(content_h * 0.5 - scaled.height() / 2)
        p.drawImage(x - 12, y - 12, _black_shadow(scaled))
        p.drawPixmap(x, y, scaled)
    else:
        _draw_monogram(p, name or game.name, width, content_h)
    _draw_title_bar(p, width, height, bar_h, (name or game.name).replace("/", " / "), pts)
    p.end()
    return pix
