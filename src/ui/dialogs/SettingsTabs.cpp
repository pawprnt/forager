#include "ui/dialogs/SettingsDialog.h"
#include "ui/Theme.h"
#include "ui/Style.h"
#include "ui/Icons.h"
#include "core/Config.h"
#include "services/SteamGridDB.h"
#include "ui/dialogs/SteamGridDBDialog.h"
#include <QRadioButton>
#include <QLineEdit>

using namespace theme;
using namespace theme::C;

static QVBoxLayout* makeCard(QVBoxLayout* parent) {
    auto* card = style::card();
    parent->addWidget(card);
    return static_cast<QVBoxLayout*>(card->layout());
}

QWidget* SettingsDialog::buildHeader() {
    auto* header = new QFrame();
    header->setObjectName("SettingsHeader");
    header->setStyleSheet(QStringLiteral(
        "color: %1; font-size: 17px; font-weight: 700; border-bottom: 1px solid %2; background-color: %3;")
        .arg(ACCENT_1, COLOR_3, COLOR_1));
    auto* lay = new QHBoxLayout(header);
    lay->setContentsMargins(20, 12, 20, 12);
    lay->setSpacing(10);

    auto* icon = new QLabel();
    icon->setPixmap(icons::loadIcon("settings", ACCENT_1).pixmap(20, 20));
    lay->addWidget(icon);

    auto* title = style::heading("Settings", 17, nullptr, 700, ACCENT_1);
    lay->addWidget(title);

    auto* subtitle = new QLabel("Library, Proton and account");
    style::label(subtitle, TEXT_DIM, 12);
    lay->addWidget(subtitle);
    lay->addStretch(1);
    return header;
}

QWidget* SettingsDialog::buildNav() {
    auto* panel = new QFrame();
    panel->setObjectName("SettingsNav");
    style::panel(panel, 2);
    panel->setFixedWidth(190);
    auto* lay = new QVBoxLayout(panel);
    lay->setContentsMargins(8, 8, 8, 8);
    lay->setSpacing(4);

    struct NavItem { QString label; QString icon; };
    NavItem items[] = {{"Library", "folder"}, {"Proton", "shield"}, {"Account", "user"}};

    m_navGroup = new QButtonGroup(this);
    m_navGroup->setExclusive(true);

    for (const auto& item : items) {
        auto* btn = navButton(item.label, item.icon);
        lay->addWidget(btn);
        m_navGroup->addButton(btn);
        if (item.label == "Library") btn->setChecked(true);
        connect(btn, &QPushButton::clicked, this, [this, i = m_navGroup->buttons().size() - 1]() {
            switchTab(i);
        });
    }
    lay->addStretch(1);
    return panel;
}

QPushButton* SettingsDialog::navButton(const QString& text, const QString& icon) {
    auto* btn = style::button(text, "navside");
    btn->setCheckable(true);
    btn->setIcon(icons::loadIcon(icon, TEXT_FAINT));
    btn->setIconSize(QSize(18, 18));
    return btn;
}

QWidget* SettingsDialog::buildFooter() {
    auto* footer = new QFrame();
    footer->setObjectName("SettingsFooter");
    footer->setStyleSheet(QStringLiteral(
        "QFrame#SettingsFooter { border-top: 1px solid %1; background: transparent; }").arg(COLOR_3));
    auto* lay = new QHBoxLayout(footer);
    lay->setContentsMargins(20, 12, 20, 14);
    lay->setSpacing(10);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    buttons->setStyleSheet(style::dialogButtonsQss());
    auto* saveBtn = buttons->button(QDialogButtonBox::Save);
    saveBtn->setObjectName("saveButton");
    saveBtn->setText("Save");
    saveBtn->setIcon(icons::loadIcon("floppy-disk", TEXT));
    saveBtn->setIconSize(QSize(14, 14));
    auto* cancelBtn = buttons->button(QDialogButtonBox::Cancel);
    cancelBtn->setText("Cancel");
    cancelBtn->setIcon(icons::loadIcon("xmark", TEXT_DIM));
    cancelBtn->setIconSize(QSize(14, 14));
    connect(buttons, &QDialogButtonBox::accepted, this, &SettingsDialog::save);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    lay->addStretch(1);
    lay->addWidget(buttons);
    return footer;
}

QWidget* SettingsDialog::buildLibraryTab() {
    auto* tab = new QWidget();
    style::transparent(tab);
    auto* libLay = style::vbox(tab, 14);
    {
        auto* cardLay = makeCard(libLay);

        auto* dirLabel = new QLabel("Game library folder");
        style::label(dirLabel, TEXT_DIM, 11);
        cardLay->addWidget(dirLabel);
        m_dirEdit = new QLineEdit(Config::instance().gamesDir());
        m_dirEdit->setStyleSheet(style::lineeditQss());
        cardLay->addWidget(m_dirEdit);

        auto* cacheLabel = new QLabel("Steam appcache/librarycache");
        style::label(cacheLabel, TEXT_DIM, 11);
        cardLay->addWidget(cacheLabel);
        m_cacheEdit = new QLineEdit(Config::instance().steamAppcache());
        m_cacheEdit->setStyleSheet(style::lineeditQss());
        cardLay->addWidget(m_cacheEdit);
    }
    {
        auto* cardLay = makeCard(libLay);

        auto* sizeLabel = new QLabel("Display size");
        style::label(sizeLabel, ACCENT_1, 11, 700);
        cardLay->addWidget(sizeLabel);

        QString currentSize = Config::instance().displaySize();
        for (const auto& s : DISPLAY_SIZES) {
            auto* rb = new QRadioButton(QStringLiteral("%1  (%2x%3)").arg(s.label).arg(s.width).arg(s.height));
            rb->setChecked(s.key == currentSize);
            cardLay->addWidget(rb);
        }
        cardLay->addStretch(1);
    }
    libLay->addStretch(1);
    return tab;
}

QWidget* SettingsDialog::buildProtonTab() {
    auto* tab = new QWidget();
    style::transparent(tab);
    auto* protonLay = style::vbox(tab, 14);
    {
        auto* cardLay = makeCard(protonLay);

        auto* prefixLabel = new QLabel("Proton prefix");
        style::label(prefixLabel, TEXT_DIM, 11);
        cardLay->addWidget(prefixLabel);
        auto* prefixStatus = new QLabel("Proton not installed");
        style::label(prefixStatus, TEXT_DIM, 12);
        cardLay->addWidget(prefixStatus);
    }
    auto* updateBtn = style::button("Update Proton…", "primary");
    connect(updateBtn, &QPushButton::clicked, this, &SettingsDialog::updateProtonRequested);
    protonLay->addWidget(updateBtn);
    protonLay->addStretch(1);
    return tab;
}

QWidget* SettingsDialog::buildAccountTab() {
    auto* tab = new QWidget();
    style::transparent(tab);
    auto* accLay = style::vbox(tab, 14);
    {
        auto* cardLay = makeCard(accLay);

        auto* steamLabel = new QLabel("Steam account");
        style::label(steamLabel, ACCENT_1, 11, 700);
        cardLay->addWidget(steamLabel);

        auto* steamStatus = new QLabel("Not signed in.");
        style::label(steamStatus, TEXT_DIM, 12);
        cardLay->addWidget(steamStatus);

        auto* steamBtns = new QHBoxLayout();
        steamBtns->setSpacing(8);
        auto* signInBtn = style::button("Sign in with Steam", "primary");
        auto* signOutBtn = style::button("Sign out", "secondary");
        steamBtns->addWidget(signInBtn);
        steamBtns->addWidget(signOutBtn);
        steamBtns->addStretch(1);
        cardLay->addLayout(steamBtns);
    }
    {
        auto* cardLay = makeCard(accLay);

        auto* sgdbLabel = new QLabel("SteamGridDB");
        style::label(sgdbLabel, ACCENT_1, 11, 700);
        cardLay->addWidget(sgdbLabel);

        auto* tokenRow = new QHBoxLayout();
        tokenRow->setSpacing(8);
        auto* tokenEdit = new QLineEdit();
        tokenEdit->setEchoMode(QLineEdit::Password);
        tokenEdit->setStyleSheet(style::lineeditQss());
        tokenEdit->setPlaceholderText("No API token set");
        if (SteamGridDB::instance().isConfigured())
            tokenEdit->setText(SteamGridDB::instance().token());
        tokenRow->addWidget(tokenEdit, 1);
        auto* tokenSaveBtn = style::button("Save token", "secondary");
        connect(tokenSaveBtn, &QPushButton::clicked, this, [tokenEdit]() {
            SteamGridDB::instance().setToken(tokenEdit->text().trimmed());
        });
        tokenRow->addWidget(tokenSaveBtn);
        cardLay->addLayout(tokenRow);

        auto* getTokenBtn = style::button("Get token", "primary");
        connect(getTokenBtn, &QPushButton::clicked, this, [this]() {
            SteamGridDBTokenDialog dlg(this);
            dlg.exec();
        });
        cardLay->addWidget(getTokenBtn, 0, Qt::AlignLeft);
    }
    accLay->addStretch(1);
    return tab;
}

void SettingsDialog::switchTab(int index) {
    m_pages->setCurrentIndex(index);
}
