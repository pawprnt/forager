#include "ui/dialogs/SteamGridDBDialog.h"
#include "ui/Theme.h"
#include "ui/Style.h"
#include <QCloseEvent>

using namespace theme::C;

static const QUrl LOGIN_URL("https://www.steamgriddb.com/login");
static const QUrl API_URL("https://www.steamgriddb.com/profile/preferences/api");

static const QString POLL_JS = QStringLiteral(
    "window.__foragerSgdbLoggedIn = false;"
    "fetch('/api/public/user', { method: 'GET', credentials: 'same-origin' })"
    "    .then(function (r) { return r.json(); })"
    "    .then(function (d) {"
    "        window.__foragerSgdbLoggedIn = !!(d && d.success === true);"
    "    })"
    "    .catch(function () { window.__foragerSgdbLoggedIn = false; });"
);

static const QString READ_JS = "window.__foragerSgdbLoggedIn === true;";

SteamGridDBTokenDialog::SteamGridDBTokenDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("SteamGridDB API token");
    setModal(true);
    resize(900, 640);
    setMinimumSize(640, 480);
    style::background(this, COLOR_1);

    auto* lay = style::vbox(this);

    m_view = new QWebEngineView(this);
    lay->addWidget(m_view, 1);

    m_status = new QLabel(
        "Sign in with Steam to create or access your SteamGridDB account \u2014 "
        "it's free and takes one click. We'll jump to your API key page next.");
    m_status->setObjectName("sgdbStatus");
    m_status->setWordWrap(true);
    style::label(m_status, TEXT_DIM, 11, -1,
        QStringLiteral("background: %1; padding: 6px 12px;").arg(COLOR_2));
    lay->addWidget(m_status);

    connect(m_view, &QWebEngineView::loadFinished, this, &SteamGridDBTokenDialog::onLoadFinished);
    m_view->load(LOGIN_URL);

    m_timer = new QTimer(this);
    m_timer->setInterval(1500);
    connect(m_timer, &QTimer::timeout, this, &SteamGridDBTokenDialog::poll);
    m_timer->start();
    poll();
}

void SteamGridDBTokenDialog::poll() {
    m_view->page()->runJavaScript(POLL_JS);
    QTimer::singleShot(700, this, &SteamGridDBTokenDialog::readPoll);
}

void SteamGridDBTokenDialog::readPoll() {
    m_view->page()->runJavaScript(READ_JS, [this](const QVariant& result) {
        onPollResult(result);
    });
}

void SteamGridDBTokenDialog::onPollResult(const QVariant& result) {
    bool loggedIn = result.toBool();
    if (loggedIn && !m_navigated) {
        m_navigated = true;
        m_status->setText("Signed in! Opening your API key page\u2026");
        m_view->load(API_URL);
    }
}

void SteamGridDBTokenDialog::onLoadFinished(bool ok) {
    if (m_onApiPage) return;
    if (m_navigated && ok) {
        m_onApiPage = true;
        m_status->setText(
            "Click the 'Create API Key' button, copy the API key, then paste "
            "it into the 'API Token' field under the SteamGridDB menu. You "
            "can close this window once it's saved.");
    } else if (!ok && !m_navigated) {
        m_status->setText("Could not load SteamGridDB \u2014 check your connection.");
    }
}

void SteamGridDBTokenDialog::cancel() {
    reject();
}

void SteamGridDBTokenDialog::closeEvent(QCloseEvent* event) {
    if (m_timer) m_timer->stop();
    if (m_view) m_view->stop();
    QDialog::closeEvent(event);
}
