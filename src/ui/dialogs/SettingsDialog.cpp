#include "ui/dialogs/SettingsDialog.h"
#include "ui/Theme.h"
#include "ui/Style.h"
#include "ui/Icons.h"
#include "ui/Fonts.h"
#include "core/Config.h"
#include <QRadioButton>
#include <QCheckBox>
#include <QLineEdit>
#include <QFileDialog>

using namespace theme;
using namespace theme::C;

static const QString NAV_BTN_QSS = QStringLiteral(
    "QPushButton { background: transparent; color: #b8bcbf; border: none; "
    "border-left: 3px solid transparent; border-radius: 6px; padding: 9px 12px; "
    "padding-left: 13px; font-size: 13px; text-align: left; } "
    "QPushButton:hover { color: %1; background-color: %2; } "
    "QPushButton:checked { color: %3; background-color: %2; font-weight: 600; "
    "border-left: 3px solid %3; padding-left: 10px; }")
    .arg(TEXT, COLOR_3, ACCENT_1);

static const QString BUTTONS_QSS = QStringLiteral(
    "QPushButton { background-color: %1; color: %2; border: 1px solid %3; "
    "border-radius: %4px; padding: 0 18px; min-height: 32px; font-size: 14px; font-weight: 600; } "
    "QPushButton:hover { background-color: %3; } "
    "QPushButton#saveButton { background-color: %5; color: %2; border: none; } "
    "QPushButton#saveButton:hover { background-color: %6; }")
    .arg(COLOR_2, TEXT, COLOR_3).arg(RADIUS).arg(ACCENT_1, ACCENT_2);

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Settings");
    resize(760, 560);
    setMinimumSize(640, 500);
    setStyleSheet(QStringLiteral("#SettingsDialog { background: %1; }").arg(COLOR_2));
    setObjectName("SettingsDialog");

    auto* v = new QVBoxLayout(this);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(0);

    v->addWidget(buildHeader());

    auto* body = new QHBoxLayout();
    body->setContentsMargins(16, 16, 16, 12);
    body->setSpacing(16);
    body->addWidget(buildNav());

    m_pages = new QStackedWidget();
    m_pages->setStyleSheet("QStackedWidget { background: transparent; }");

    // Library tab
    m_libraryTab = new QWidget();
    m_libraryTab->setStyleSheet("background: transparent;");
    auto* libLay = new QVBoxLayout(m_libraryTab);
    libLay->setContentsMargins(0, 0, 0, 0);
    libLay->setSpacing(14);
    {
        auto* card = new QFrame();
        card->setStyleSheet(QStringLiteral("background-color: %1; border-radius: %2px;").arg(COLOR_2).arg(RADIUS));
        auto* cardLay = new QVBoxLayout(card);
        cardLay->setContentsMargins(12, 12, 12, 12);
        cardLay->setSpacing(8);

        auto* dirLabel = new QLabel("Game library folder");
        style::label(dirLabel, TEXT_DIM, 11);
        cardLay->addWidget(dirLabel);
        auto* dirEdit = new QLineEdit(Config::instance().gamesDir());
        dirEdit->setStyleSheet(style::lineeditQss());
        cardLay->addWidget(dirEdit);

        auto* cacheLabel = new QLabel("Steam appcache/librarycache");
        style::label(cacheLabel, TEXT_DIM, 11);
        cardLay->addWidget(cacheLabel);
        auto* cacheEdit = new QLineEdit(Config::instance().steamAppcache());
        cacheEdit->setStyleSheet(style::lineeditQss());
        cardLay->addWidget(cacheEdit);

        libLay->addWidget(card);
    }
    {
        auto* card = new QFrame();
        card->setStyleSheet(QStringLiteral("background-color: %1; border-radius: %2px;").arg(COLOR_2).arg(RADIUS));
        auto* cardLay = new QVBoxLayout(card);
        cardLay->setContentsMargins(12, 12, 12, 12);
        cardLay->setSpacing(8);

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
        libLay->addWidget(card);
    }
    libLay->addStretch(1);
    m_pages->addWidget(m_libraryTab);

    // Proton tab
    m_protonTab = new QWidget();
    m_protonTab->setStyleSheet("background: transparent;");
    auto* protonLay = new QVBoxLayout(m_protonTab);
    protonLay->setContentsMargins(0, 0, 0, 0);
    protonLay->setSpacing(14);
    {
        auto* card = new QFrame();
        card->setStyleSheet(QStringLiteral("background-color: %1; border-radius: %2px;").arg(COLOR_2).arg(RADIUS));
        auto* cardLay = new QVBoxLayout(card);
        cardLay->setContentsMargins(12, 12, 12, 12);
        cardLay->setSpacing(8);

        auto* prefixLabel = new QLabel("Proton prefix");
        style::label(prefixLabel, TEXT_DIM, 11);
        cardLay->addWidget(prefixLabel);
        auto* prefixStatus = new QLabel("Proton not installed");
        style::label(prefixStatus, TEXT_DIM, 12);
        cardLay->addWidget(prefixStatus);
        protonLay->addWidget(card);
    }
    auto* updateBtn = new QPushButton("Update Proton\u2026");
    updateBtn->setCursor(Qt::PointingHandCursor);
    updateBtn->setStyleSheet(style::buttonQss("primary"));
    connect(updateBtn, &QPushButton::clicked, this, &SettingsDialog::updateProtonRequested);
    protonLay->addWidget(updateBtn);
    protonLay->addStretch(1);
    m_pages->addWidget(m_protonTab);

    // Account tab
    m_accountTab = new QWidget();
    m_accountTab->setStyleSheet("background: transparent;");
    auto* accLay = new QVBoxLayout(m_accountTab);
    accLay->setContentsMargins(0, 0, 0, 0);
    accLay->setSpacing(14);
    {
        auto* card = new QFrame();
        card->setStyleSheet(QStringLiteral("background-color: %1; border-radius: %2px;").arg(COLOR_2).arg(RADIUS));
        auto* cardLay = new QVBoxLayout(card);
        cardLay->setContentsMargins(12, 12, 12, 12);
        cardLay->setSpacing(8);

        auto* steamLabel = new QLabel("Steam account");
        style::label(steamLabel, ACCENT_1, 11, 700);
        cardLay->addWidget(steamLabel);

        auto* steamStatus = new QLabel("Not signed in.");
        style::label(steamStatus, TEXT_DIM, 12);
        cardLay->addWidget(steamStatus);

        auto* steamBtns = new QHBoxLayout();
        steamBtns->setSpacing(8);
        auto* signInBtn = new QPushButton("Sign in with Steam");
        signInBtn->setCursor(Qt::PointingHandCursor);
        signInBtn->setStyleSheet(style::buttonQss("primary"));
        auto* signOutBtn = new QPushButton("Sign out");
        signOutBtn->setCursor(Qt::PointingHandCursor);
        signOutBtn->setStyleSheet(style::buttonQss("secondary"));
        steamBtns->addWidget(signInBtn);
        steamBtns->addWidget(signOutBtn);
        steamBtns->addStretch(1);
        cardLay->addLayout(steamBtns);
        accLay->addWidget(card);
    }
    {
        auto* card = new QFrame();
        card->setStyleSheet(QStringLiteral("background-color: %1; border-radius: %2px;").arg(COLOR_2).arg(RADIUS));
        auto* cardLay = new QVBoxLayout(card);
        cardLay->setContentsMargins(12, 12, 12, 12);
        cardLay->setSpacing(8);

        auto* sgdbLabel = new QLabel("SteamGridDB");
        style::label(sgdbLabel, ACCENT_1, 11, 700);
        cardLay->addWidget(sgdbLabel);

        auto* tokenRow = new QHBoxLayout();
        tokenRow->setSpacing(8);
        auto* tokenEdit = new QLineEdit();
        tokenEdit->setEchoMode(QLineEdit::Password);
        tokenEdit->setStyleSheet(style::lineeditQss());
        tokenEdit->setPlaceholderText("No API token set");
        tokenRow->addWidget(tokenEdit, 1);
        auto* tokenSaveBtn = new QPushButton("Save token");
        tokenSaveBtn->setCursor(Qt::PointingHandCursor);
        tokenSaveBtn->setStyleSheet(style::buttonQss("secondary"));
        tokenRow->addWidget(tokenSaveBtn);
        cardLay->addLayout(tokenRow);

        auto* getTokenBtn = new QPushButton("Get token");
        getTokenBtn->setCursor(Qt::PointingHandCursor);
        getTokenBtn->setStyleSheet(style::buttonQss("primary"));
        cardLay->addWidget(getTokenBtn, 0, Qt::AlignLeft);
        accLay->addWidget(card);
    }
    accLay->addStretch(1);
    m_pages->addWidget(m_accountTab);

    auto* content = new QFrame();
    content->setObjectName("SettingsContent");
    style::panel(content, 2);
    auto* contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(20, 20, 20, 20);
    contentLayout->addWidget(m_pages);
    body->addWidget(content, 1);

    v->addLayout(body);
    v->addWidget(buildFooter());
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

    auto* title = new QLabel("Settings");
    QFont titleFont(fonts::UI_FONT, 17, QFont::Bold);
    title->setFont(titleFont);
    style::label(title, ACCENT_1);
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

    m_navList = new QListWidget();
    m_navList->setStyleSheet("QListWidget { background: transparent; border: none; }");
    for (const auto& item : items) {
        auto* btn = new QPushButton(item.label);
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setIcon(icons::loadIcon(item.icon, "#b8bcbf"));
        btn->setIconSize(QSize(18, 18));
        btn->setStyleSheet(NAV_BTN_QSS);
        lay->addWidget(btn);
        if (item.label == "Library") btn->setChecked(true);
        connect(btn, &QPushButton::clicked, this, [this, btn]() {
            int idx = 0;
            if (btn->text() == "Library") idx = 0;
            else if (btn->text() == "Proton") idx = 1;
            else idx = 2;
            switchTab(idx);
        });
    }
    lay->addStretch(1);
    return panel;
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
    buttons->setStyleSheet(BUTTONS_QSS);
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

void SettingsDialog::switchTab(int index) {
    m_pages->setCurrentIndex(index);
}

QString SettingsDialog::selectedCardSize() const {
    return Config::instance().displaySize();
}

QString SettingsDialog::gamesDirText() const {
    return Config::instance().gamesDir();
}

void SettingsDialog::done(int result) {
    QDialog::done(result);
}

void SettingsDialog::save() {
    accept();
}
