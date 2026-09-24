#include "ui/widgets/TitleBar.h"

#include <QApplication>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QToolButton>

#include "ui/Fonts.h"
#include "ui/Icons.h"
#include "ui/Style.h"
#include "ui/Theme.h"

using namespace theme;
using namespace theme::C;

static QPushButton* navTabButton(const QString& text, QButtonGroup* group) {
    auto* btn = style::button(text, "navtab");
    btn->setCheckable(true);
    group->addButton(btn);
    return btn;
}

TitleBar::TitleBar(QWidget* parent)
    : QWidget(parent)
{
    setFixedHeight(52);
    setStyleSheet(QStringLiteral("background-color: %1; border-bottom: 1px solid %2;")
                      .arg(COLOR_1, COLOR_3));

    auto* lay = new QHBoxLayout(this);
    lay->setContentsMargins(16, 0, 16, 0);
    lay->setSpacing(10);

    _logo = new QToolButton();
    _logo->setText("forager");
    _logo->setFont(QFont(fonts::UI_FONT, 16, QFont::Bold));
    _logo->setCursor(Qt::PointingHandCursor);
    _logo->setToolTip("forager menu");
    _logo->setPopupMode(QToolButton::InstantPopup);
    _logo->setStyleSheet(style::toolbuttonQss());
    _mainMenu = new QMenu(this);
    _mainMenu->addAction("Settings\u2026", this, &TitleBar::settingsRequested);
    _mainMenu->addAction("Update Proton", this, &TitleBar::updateProtonRequested);
    _mainMenu->addSeparator();
    _mainMenu->addAction("Quit", qApp, &QApplication::quit);
    _logo->setMenu(_mainMenu);
    lay->addWidget(_logo);

    _backBtn = navButton("arrow-left");
    _forwardBtn = navButton("arrow-right");
    _backBtn->setToolTip("Back to Library");
    _forwardBtn->setToolTip("Forward");
    connect(_backBtn, &QPushButton::clicked, this, &TitleBar::backRequested);
    _forwardBtn->setEnabled(false);
    lay->addWidget(_backBtn);
    lay->addWidget(_forwardBtn);

    lay->addStretch(1);

    auto* tabBar = new QWidget();
    auto* tabLay = style::hbox(tabBar);
    _tabsGroup = new QButtonGroup(this);
    _tabsGroup->setExclusive(true);
    _storeTab = navTabButton("Store", _tabsGroup);
    _libraryTab = navTabButton("Library", _tabsGroup);
    tabLay->addWidget(_storeTab);
    tabLay->addWidget(_libraryTab);
    connect(_storeTab, &QPushButton::clicked, this, &TitleBar::storeTabRequested);
    connect(_libraryTab, &QPushButton::clicked, this, &TitleBar::libraryTabRequested);
    _libraryTab->setChecked(true);
    lay->addWidget(tabBar, 0, Qt::AlignCenter);

    lay->addStretch(1);

    _updatePill = style::button(QString(), "pill");
    _updatePill->hide();
    connect(_updatePill, &QPushButton::clicked, this, &TitleBar::runUpdatesRequested);
    lay->addWidget(_updatePill);

    _controllerHint = new QLabel("");
    style::label(_controllerHint, TEXT_DIM, 11, -1, "padding: 4px 8px;");
    lay->addWidget(_controllerHint);
}

QPushButton* TitleBar::navButton(const QString& iconName) {
    auto* btn = style::button(QString(), "icon");
    btn->setFixedSize(32, 32);
    btn->setIcon(icons::loadIcon(iconName, TEXT));
    btn->setIconSize(QSize(18, 18));
    return btn;
}

void TitleBar::setActiveTab(const QString& name) {
    if (name == "store")
        _storeTab->setChecked(true);
    else
        _libraryTab->setChecked(true);
}

void TitleBar::setBackEnabled(bool enabled) {
    _backBtn->setEnabled(enabled);
}

void TitleBar::setControllerHint(const QString& text) {
    _controllerHint->setText(text);
}

void TitleBar::setUpdates(int count) {
    if (count <= 0) {
        _updatePill->hide();
        return;
    }
    _updatePill->setText(QStringLiteral("%1 update%2 available")
                             .arg(count)
                             .arg(count != 1 ? QStringLiteral("s") : QString()));
    _updatePill->show();
}
