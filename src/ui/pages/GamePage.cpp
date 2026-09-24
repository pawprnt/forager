#include "ui/pages/GamePage.h"
#include "ui/widgets/Banner.h"
#include "ui/Theme.h"
#include "ui/Style.h"
#include "ui/Icons.h"
#include "ui/Fonts.h"
#include <QScrollBar>
#include <QFont>

using namespace theme::C;

static constexpr int BANNER_H = 420;

GamePage::GamePage(QWidget* parent)
    : QWidget(parent)
{
    style::background(this, COLOR_1);

    auto* outer = style::vbox(this, 0);
    outer->setContentsMargins(16, 16, 16, 16);

    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet(QStringLiteral("QScrollArea { background-color: %1; border: none; }").arg(COLOR_1));

    auto* content = new QWidget();
    style::background(content, COLOR_1);
    auto* v = new QVBoxLayout(content);
    v->setContentsMargins(0, 0, 0, 16);
    v->setSpacing(16);

    m_banner = new Banner(content);
    v->addWidget(m_banner);

    m_title = style::heading("", 26, content);
    m_title->setWordWrap(true);
    v->addWidget(m_title);

    auto* infoRow = new QHBoxLayout();
    infoRow->setSpacing(12);

    m_playBtn = style::button(QString(), "play");
    m_playBtn->setFixedHeight(48);
    m_playBtn->setMinimumWidth(220);
    auto* playLay = new QHBoxLayout(m_playBtn);
    playLay->setContentsMargins(20, 0, 16, 0);
    playLay->setSpacing(5);
    m_playIconLabel = new QLabel();
    m_playIconLabel->setPixmap(icons::loadIcon("play", "#ffffff").pixmap(20, 20));
    style::transparent(m_playIconLabel);
    playLay->addWidget(m_playIconLabel);
    m_playText = new QLabel("Play");
    style::label(m_playText, "#ffffff", 17, 600);
    playLay->addWidget(m_playText);
    playLay->addStretch(1);
    connect(m_playBtn, &QPushButton::clicked, this, &GamePage::onPlayClicked);
    infoRow->addWidget(m_playBtn);

    m_sourceBadge = new QLabel();
    style::label(m_sourceBadge, TEXT_DIM, 12, -1,
        QStringLiteral("background-color: %1; padding: 6px 12px; border-radius: %2px;").arg(COLOR_3).arg(RADIUS));
    infoRow->addWidget(m_sourceBadge, 0, Qt::AlignVCenter);

    m_pathLabel = new QLabel();
    style::label(m_pathLabel, TEXT_DIM, 12);
    m_pathLabel->setWordWrap(true);
    infoRow->addWidget(m_pathLabel, 1);

    v->addLayout(infoRow);

    m_infoBox = buildInfoBox();
    v->addWidget(m_infoBox, 0, Qt::AlignLeft);

    m_achFrame = buildAchievementsBox();
    m_achFrame->hide();
    v->addWidget(m_achFrame);

    v->addStretch(1);

    scroll->setWidget(content);
    outer->addWidget(scroll);

    m_banner->setOverlay(buildBannerOverlay());
}

QWidget* GamePage::buildBannerOverlay() {
    auto* overlay = new QWidget(m_banner);
    style::transparent(overlay);

    auto* lay = new QVBoxLayout(overlay);
    lay->setContentsMargins(20, 16, 20, 20);
    lay->setSpacing(12);

    auto* top = new QHBoxLayout();
    auto* backBtn = style::button("‹  Library", "overlay");
    connect(backBtn, &QPushButton::clicked, this, &GamePage::backRequested);
    top->addWidget(backBtn);
    top->addStretch(1);
    lay->addLayout(top);

    lay->addStretch(1);

    m_logoLabel = new QLabel();
    m_logoLabel->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
    m_logoLabel->setMaximumWidth(520);
    m_logoLabel->setMinimumHeight(90);
    lay->addWidget(m_logoLabel);

    return overlay;
}

QFrame* GamePage::makeBox(const QString& title, QVBoxLayout** outLay, QLabel** outHeader) {
    auto* box = style::card(nullptr, 2, 14, 12, 10);
    auto* v = static_cast<QVBoxLayout*>(box->layout());
    auto* header = new QLabel(title);
    style::label(header, TEXT_DIM, 11, 700);
    v->addWidget(header);
    if (outLay) *outLay = v;
    if (outHeader) *outHeader = header;
    return box;
}

QFrame* GamePage::buildInfoBox() {
    QVBoxLayout* v = nullptr;
    auto* box = makeBox("GAME INFO", &v);
    box->setFixedWidth(300);

    auto buildRow = [&](const QString& key, QLabel*& valLabel) {
        auto* row = new QHBoxLayout();
        row->setSpacing(8);
        auto* k = new QLabel(key.toUpper());
        style::label(k, TEXT_DIM, 11);
        k->setFixedWidth(70);
        valLabel = new QLabel();
        style::label(valLabel, TEXT, 12);
        valLabel->setWordWrap(true);
        row->addWidget(k);
        row->addWidget(valLabel, 1);
        v->addLayout(row);
    };

    buildRow("Source", m_infoSource);
    buildRow("App ID", m_infoAppId);

    v->addStretch(1);
    return box;
}

QFrame* GamePage::buildAchievementsBox() {
    QVBoxLayout* v = nullptr;
    auto* box = makeBox("ACHIEVEMENTS", &v, &m_achHeader);

    m_achList = new QListWidget();
    m_achList->setStyleSheet(style::listQss());
    m_achList->setMaximumHeight(180);
    v->addWidget(m_achList);
    return box;
}

void GamePage::populateAchievements(const Game& game) {
    Q_UNUSED(game);
    m_achFrame->hide();
}

void GamePage::setGame(const Game& game) {
    m_game = game;
    m_running = false;

    if (m_logo.isNull()) {
        m_title->setText(QString(game.name()).replace("/", " / "));
        m_logoLabel->clear();
    }

    m_sourceBadge->setText(game.sourceName());
    m_pathLabel->setText(game.displayPath());
    m_infoSource->setText(game.sourceName());
    m_infoAppId->setText(game.appId().isEmpty() ? "\u2014" : game.appId());

    updatePlayButton();
    populateAchievements(game);
}

void GamePage::setRunning(bool running) {
    if (m_game.name().isEmpty()) return;
    if (!m_game.isInstalled()) return;
    if (running == m_running) return;
    m_running = running;
    updatePlayButton();
}

void GamePage::setHero(const QPixmap& pix) {
    if (pix.isNull()) return;
    m_banner->setSource(pix);
}

void GamePage::updatePlayButton() {
    bool installed = m_game.isInstalled() && !m_game.path().isEmpty();
    if (installed) {
        m_playBtn->setStyleSheet(style::buttonQss(m_running ? "running" : "play"));
        m_playIconLabel->setPixmap(icons::loadIcon(m_running ? "stop" : "play", "#ffffff").pixmap(20, 20));
        m_playText->setText(m_running ? "Stop" : "Play");
    } else {
        m_playBtn->setStyleSheet(style::buttonQss("play"));
        m_playIconLabel->setPixmap(icons::loadIcon("box", "#ffffff").pixmap(20, 20));
        m_playText->setText("Install");
    }
}

void GamePage::onPlayClicked() {
    if (m_game.name().isEmpty()) return;
    if (!m_game.isInstalled() || m_game.path().isEmpty()) {
        emit install(m_game);
        return;
    }
    if (m_running) {
        emit stop(m_game);
    } else {
        emit play(m_game);
    }
}
