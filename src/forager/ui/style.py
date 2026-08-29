from __future__ import annotations

from PySide6.QtWidgets import QWidget, QLabel, QPushButton

from forager.ui.theme import C


RADIUS = C.RADIUS


def surface(level: int = 2) -> str:
    """Background-color fragment for a SpaceTheme surface elevation step."""
    return f"background-color: {getattr(C, 'COLOR_' + str(level))};"


def surface_qss(level: int = 2, radius: int = RADIUS) -> str:
    """Box surface: filled surface color at the given elevation, rounded."""
    return f"background-color: {getattr(C, 'COLOR_' + str(level))}; border-radius: {radius}px;"


def text(color: str = C.TEXT, transparent: bool = True) -> str:
    """`color:` fragment, optionally clearing the widget background."""
    bg = " background: transparent;" if transparent else ""
    return f"color: {color};{bg}"


def label(
    widget: QLabel,
    color: str = C.TEXT,
    size: int | None = None,
    weight: int | None = None,
    transparent: bool = True,
    padding: str | None = None,
) -> None:
    """Apply SpaceTheme label styling to ``widget`` in place."""
    parts = [f"color: {color};"]
    if transparent:
        parts.append("background: transparent;")
    if size is not None:
        parts.append(f"font-size: {size}px;")
    if weight is not None:
        parts.append(f"font-weight: {weight};")
    if padding is not None:
        parts.append(f"padding: {padding};")
    widget.setStyleSheet("".join(parts))


def panel(widget: QWidget, level: int = 2, radius: int = RADIUS) -> None:
    """Style ``widget`` as a SpaceTheme shelf/panel surface."""
    widget.setStyleSheet(surface_qss(level, radius))


def rounded(widget: QWidget, radius: int = RADIUS) -> None:
    widget.setStyleSheet(f"border-radius: {radius}px;")


def lineedit_qss(
    bg: str = C.COLOR_3,
    radius: int = RADIUS,
    focus: str = C.ACCENT_1,
    padding: str = "7px 12px",
    size: int = 13,
) -> str:
    """SpaceTheme search/edit field: flat filled surface, accent focus ring."""
    return (
        f"QLineEdit {{ background-color: {bg}; border: none; border-radius: {radius}px; "
        f"padding: {padding}; font-size: {size}px; }}"
        f"QLineEdit:focus {{ border: 1px solid {focus}; }}"
    )


def button_qss(kind: str = "primary", radius: int = RADIUS) -> str:
    """Reusable button QSS for the common SpaceTheme button variants.

    Kinds: ``play`` (green launch), ``running`` (greyed), ``primary`` (accent),
    ``secondary`` (outlined surface), ``ghost`` (transparent accent text).
    """
    base = "QPushButton { border: none; border-radius: %dpx;" % radius
    if kind == "play":
        return (
            f"{base} background-color: {C.GREEN}; color: {C.TEXT}; }} "
            f"QPushButton:hover {{ background-color: {C.GREEN_HOVER}; }} "
            f"QPushButton:disabled {{ background-color: {C.COLOR_3}; color: {C.TEXT_DIM}; }}"
        )
    if kind == "running":
        return (
            f"{base} background-color: {C.COLOR_3}; color: {C.TEXT}; }} "
            f"QPushButton:hover {{ background-color: {C.COLOR_4}; }}"
        )
    if kind == "primary":
        return (
            f"{base} background-color: {C.ACCENT_1}; color: {C.TEXT}; }} "
            f"QPushButton:hover {{ background-color: {C.ACCENT_2}; }} "
            f"QPushButton:disabled {{ background-color: {C.COLOR_2}; color: {C.TEXT_DIM}; }}"
        )
    if kind == "secondary":
        return (
            f"{base} background-color: {C.COLOR_2}; color: {C.TEXT}; "
            f"border: 1px solid {C.COLOR_3}; }} "
            f"QPushButton:hover {{ background-color: {C.COLOR_3}; }}"
        )
    if kind == "ghost":
        return (
            "QPushButton { background: transparent; color: "
            f"{C.ACCENT_1}; border: none; }} "
            f"QPushButton:hover {{ color: {C.ACCENT_2}; }}"
        )
    return ""


def icon_button_qss(bg: str = C.COLOR_2, hover: str = C.COLOR_3, radius: int = RADIUS) -> str:
    """Square icon button: flat surface that lightens on hover."""
    return (
        f"QPushButton {{ background-color: {bg}; border: none; border-radius: {radius}px; }} "
        f"QPushButton:hover {{ background-color: {hover}; }}"
    )


def pill_qss(color: str = C.YELLOW, hover: str = C.COLOR_3, radius: int = RADIUS) -> str:
    """Outlined status pill (e.g. the Updates Available badge)."""
    return (
        f"QPushButton {{ color: {color}; background: transparent; "
        f"border: 1px solid {color}; border-radius: {radius}px; "
        f"padding: 4px 10px; font-size: 11px; }} "
        f"QPushButton:hover {{ background-color: {hover}; }}"
    )


def toolbutton_qss(color: str = C.ACCENT_1, hover: str = C.COLOR_3, radius: int = RADIUS) -> str:
    """Transparent accent tool-button (e.g. the logo menu button)."""
    return (
        f"QToolButton {{ color: {color}; background: transparent; border: none; "
        f"border-radius: {radius}px; padding: 6px 10px; }} "
        f"QToolButton:hover {{ background-color: {hover}; }} "
        f"QToolButton::menu-indicator {{ image: none; }}"
    )
