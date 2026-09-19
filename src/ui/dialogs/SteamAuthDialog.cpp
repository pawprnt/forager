#include "ui/dialogs/SteamAuthDialog.h"
#include "ui/Theme.h"
#include "ui/Style.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>
#include <QImage>
#include <QThread>
#include <QCloseEvent>
#include <QMutexLocker>
#include <QPainter>
#include <QEventLoop>
#include <QTimer>

using namespace theme;
using namespace theme::C;

static const int QR_PIXEL_SIZE = 256;

static const QString INPUT_QSS = QStringLiteral(
    "QLineEdit { background-color: %1; border: none; border-radius: %2px; "
    "padding: 7px 12px; font-size: 13px; } "
    "QLineEdit:focus { border: 1px solid %3; }")
    .arg(COLOR_3).arg(RADIUS).arg(ACCENT_1);

static const QString PRIMARY_BTN_QSS = QStringLiteral(
    "QPushButton { background-color: %1; color: %2; border: none; border-radius: %3px; "
    "padding: 8px 16px; font-size: 13px; font-weight: 600; } "
    "QPushButton:hover { background-color: %4; }")
    .arg(ACCENT_1, TEXT).arg(RADIUS).arg(ACCENT_2);

static const QString SECONDARY_BTN_QSS = QStringLiteral(
    "QPushButton { background-color: %1; color: %2; border: 1px solid %3; "
    "border-radius: %4px; padding: 8px 16px; font-size: 13px; } "
    "QPushButton:hover { background-color: %5; }")
    .arg(COLOR_2, TEXT, COLOR_3).arg(RADIUS).arg(COLOR_3);

static const QString LINK_QSS = QStringLiteral(
    "QPushButton { background: transparent; color: %1; border: none; "
    "font-size: 12px; padding: 4px 0; } "
    "QPushButton:hover { color: %2; }")
    .arg(ACCENT_1, ACCENT_2);

static const QString STATUS_QSS = QStringLiteral(
    "color: %1; font-size: 11px; background: transparent;").arg(TEXT_DIM);

static const QString TITLE_QSS = QStringLiteral(
    "color: %1; font-size: 18px; font-weight: bold; background: transparent;").arg(TEXT);

// -- SteamAuthWorker -------------------------------------------------------

SteamAuthWorker::SteamAuthWorker(const QString& method, const QString& username,
                                 const QString& password, QObject* parent)
    : QThread(parent), m_method(method), m_username(username), m_password(password)
{
}

void SteamAuthWorker::submitCode(const QString& code) {
    QMutexLocker lock(&m_mutex);
    m_pendingCode = code;
    m_codeReady.wakeOne();
}

void SteamAuthWorker::cancel() {
    QMutexLocker lock(&m_mutex);
    m_cancelled = true;
    m_codeReady.wakeOne();
}

void SteamAuthWorker::run() {
    QNetworkAccessManager nam;

    auto waitForReply = [&](QNetworkReply* reply) -> QByteArray {
        QEventLoop loop;
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        QTimer::singleShot(15000, &loop, &QEventLoop::quit);
        loop.exec();
        QByteArray data = reply->readAll();
        reply->deleteLater();
        return data;
    };

    auto postForm = [&](const QUrl& url, const QUrlQuery& params) -> QJsonObject {
        QNetworkRequest req(url);
        req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        QNetworkReply* reply = nam.post(req, params.toString(QUrl::FullyEncoded).toUtf8());
        return QJsonDocument::fromJson(waitForReply(reply)).object();
    };

    auto postJson = [&](const QUrl& url, const QJsonObject& body) -> QJsonObject {
        QNetworkRequest req(url);
        req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        QNetworkReply* reply = nam.post(req, QJsonDocument(body).toJson());
        return QJsonDocument::fromJson(waitForReply(reply)).object();
    };

    auto httpGet = [&](const QUrl& url) -> QJsonObject {
        QNetworkReply* reply = nam.get(QNetworkRequest(url));
        return QJsonDocument::fromJson(waitForReply(reply)).object();
    };

    auto isCancelled = [&]() -> bool {
        QMutexLocker lock(&m_mutex);
        return m_cancelled;
    };

    auto drainCode = [&]() -> QString {
        QMutexLocker lock(&m_mutex);
        QString code = m_pendingCode;
        m_pendingCode.clear();
        return code;
    };

    try {
        QString challengeUrl;
        QString clientId;
        QString requestId;
        int interval = 5;

        if (m_method == "qr") {
            QJsonObject dev;
            dev["device_name"] = "Forager";
            dev["device_type"] = 1;
            dev["os"] = "linux";
            QJsonObject body;
            body["device_details"] = dev;
            QJsonObject resp = postJson(
                QUrl("https://api.steampowered.com/IAuthenticationService/BeginAuthSessionViaQR/v1/"), body);
            QJsonObject r = resp.value("response").toObject();
            challengeUrl = r.value("challenge_url").toString();
            clientId = QString::number(static_cast<qint64>(r.value("client_id").toDouble()));
            requestId = QString::number(static_cast<qint64>(r.value("request_id").toDouble()));
            interval = r.value("interval").toInt(5);
            emit qrReady(challengeUrl);
            emit status("Scan the QR code with the Steam mobile app.");
        } else {
            if (m_username.isEmpty() || m_password.isEmpty()) {
                emit done(false, "Enter your Steam account name and password.");
                return;
            }
            QJsonObject body;
            body["account_name"] = m_username;
            QJsonObject resp = postJson(
                QUrl("https://api.steampowered.com/IAuthenticationService/BeginAuthSessionViaCredentials/v1/"), body);
            QJsonObject r = resp.value("response").toObject();
            clientId = QString::number(static_cast<qint64>(r.value("client_id").toDouble()));
            requestId = QString::number(static_cast<qint64>(r.value("request_id").toDouble()));
            interval = r.value("interval").toInt(5);
            emit status("Enter your credentials.");
        }

        while (!isCancelled()) {
            QString code = drainCode();
            if (!code.isEmpty()) {
                QUrlQuery params;
                params.addQueryItem("client_id", clientId);
                params.addQueryItem("steam_code", code);
                postForm(QUrl("https://api.steampowered.com/IAuthenticationService/UpdateAuthSessionWithSteamGuardCode/v1/"), params);
                emit status("Checking your code\u2026");
            }

            QUrlQuery pollParams;
            pollParams.addQueryItem("client_id", clientId);
            pollParams.addQueryItem("request_id", requestId);
            QJsonObject pollResp = postForm(
                QUrl("https://api.steampowered.com/IAuthenticationService/PollAuthSessionStatus/v1/"), pollParams);
            QJsonObject pollR = pollResp.value("response").toObject();
            QString refreshToken = pollR.value("refresh_token").toString();
            if (!refreshToken.isEmpty()) {
                QString accountName = pollR.value("account_name").toString();
                if (accountName.isEmpty()) accountName = m_username;
                emit done(true, accountName);
                return;
            }

            if (pollR.value("old_client_id").toDouble() > 0) {
                clientId = QString::number(static_cast<qint64>(pollR.value("old_client_id").toDouble()));
            }
            if (!pollR.value("error").toString().isEmpty()) {
                emit status(pollR.value("error").toString());
            }

            QMutexLocker lock(&m_mutex);
            if (m_cancelled) break;
            m_codeReady.wait(&m_mutex, interval * 1000);
        }

        emit done(false, "Sign-in cancelled.");
    } catch (...) {
        emit done(false, "Unexpected error during sign-in.");
    }
}

// -- SteamAuthDialog -------------------------------------------------------

SteamAuthDialog::SteamAuthDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Sign in with Steam");
    setModal(true);
    setMinimumWidth(360);
    setStyleSheet(QStringLiteral("QDialog { background-color: %1; }").arg(BG));

    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(20, 20, 20, 20);
    lay->setSpacing(10);

    m_title = new QLabel("Sign in with Steam");
    m_title->setStyleSheet(TITLE_QSS);
    lay->addWidget(m_title);

    // QR mode
    m_qrBox = new QVBoxLayout();
    m_qrBox->setSpacing(8);
    m_qrImage = new QLabel();
    m_qrImage->setAlignment(Qt::AlignCenter);
    m_qrImage->setMinimumSize(QR_PIXEL_SIZE, QR_PIXEL_SIZE);
    m_qrBox->addWidget(m_qrImage);
    m_qrHint = new QLabel("Open the Steam mobile app \u2192 Scan this QR code to approve the sign-in.");
    m_qrHint->setWordWrap(true);
    m_qrHint->setStyleSheet(STATUS_QSS);
    m_qrBox->addWidget(m_qrHint);
    m_qrRefreshBtn = new QPushButton("Refresh QR code");
    m_qrRefreshBtn->setCursor(Qt::PointingHandCursor);
    m_qrRefreshBtn->setStyleSheet(SECONDARY_BTN_QSS);
    connect(m_qrRefreshBtn, &QPushButton::clicked, this, &SteamAuthDialog::refreshQr);
    m_qrBox->addWidget(m_qrRefreshBtn);
    m_qrPasswordLink = new QPushButton("Use password instead");
    m_qrPasswordLink->setCursor(Qt::PointingHandCursor);
    m_qrPasswordLink->setStyleSheet(LINK_QSS);
    connect(m_qrPasswordLink, &QPushButton::clicked, this, &SteamAuthDialog::showPasswordMode);
    m_qrBox->addWidget(m_qrPasswordLink);
    lay->addLayout(m_qrBox);

    // Password mode
    m_pwBox = new QVBoxLayout();
    m_pwBox->setSpacing(8);
    m_pwUser = new QLineEdit();
    m_pwUser->setPlaceholderText("Steam account name");
    m_pwUser->setStyleSheet(INPUT_QSS);
    m_pwBox->addWidget(m_pwUser);
    m_pwPass = new QLineEdit();
    m_pwPass->setPlaceholderText("Password");
    m_pwPass->setEchoMode(QLineEdit::Password);
    m_pwPass->setStyleSheet(INPUT_QSS);
    m_pwBox->addWidget(m_pwPass);
    m_pwSignInBtn = new QPushButton("Sign in");
    m_pwSignInBtn->setCursor(Qt::PointingHandCursor);
    m_pwSignInBtn->setStyleSheet(PRIMARY_BTN_QSS);
    connect(m_pwSignInBtn, &QPushButton::clicked, this, &SteamAuthDialog::startPassword);
    m_pwBox->addWidget(m_pwSignInBtn);
    m_pwQrLink = new QPushButton("Use QR instead");
    m_pwQrLink->setCursor(Qt::PointingHandCursor);
    m_pwQrLink->setStyleSheet(LINK_QSS);
    connect(m_pwQrLink, &QPushButton::clicked, this, &SteamAuthDialog::showQrMode);
    m_pwBox->addWidget(m_pwQrLink);
    lay->addLayout(m_pwBox);

    // Steam Guard code row
    m_codeRow = new QHBoxLayout();
    m_codeRow->setSpacing(8);
    m_codeEdit = new QLineEdit();
    m_codeEdit->setPlaceholderText("Steam Guard code");
    m_codeEdit->setStyleSheet(INPUT_QSS);
    connect(m_codeEdit, &QLineEdit::returnPressed, this, &SteamAuthDialog::submitCode);
    m_codeEdit->setEnabled(false);
    m_codeRow->addWidget(m_codeEdit, 1);
    m_codeBtn = new QPushButton("Submit");
    m_codeBtn->setCursor(Qt::PointingHandCursor);
    m_codeBtn->setStyleSheet(PRIMARY_BTN_QSS);
    m_codeBtn->setEnabled(false);
    connect(m_codeBtn, &QPushButton::clicked, this, &SteamAuthDialog::submitCode);
    m_codeRow->addWidget(m_codeBtn);
    lay->addLayout(m_codeRow);

    m_codeHint = new QLabel();
    m_codeHint->setWordWrap(true);
    m_codeHint->setStyleSheet(STATUS_QSS);
    lay->addWidget(m_codeHint);

    m_status = new QLabel();
    m_status->setWordWrap(true);
    m_status->setStyleSheet(STATUS_QSS);
    lay->addWidget(m_status);
    lay->addStretch(1);

    showQrMode();
    startQr();
}

void SteamAuthDialog::showQrMode() {
    m_pwUser->setVisible(false);
    m_pwPass->setVisible(false);
    m_pwSignInBtn->setVisible(false);
    m_pwQrLink->setVisible(false);
    m_qrImage->setVisible(true);
    m_qrHint->setVisible(true);
    m_qrRefreshBtn->setVisible(true);
    m_qrPasswordLink->setVisible(true);
}

void SteamAuthDialog::showPasswordMode() {
    m_qrImage->setVisible(false);
    m_qrHint->setVisible(false);
    m_qrRefreshBtn->setVisible(false);
    m_qrPasswordLink->setVisible(false);
    m_pwUser->setVisible(true);
    m_pwPass->setVisible(true);
    m_pwSignInBtn->setVisible(true);
    m_pwQrLink->setVisible(true);
    m_pwUser->setFocus();
}

void SteamAuthDialog::startQr() {
    m_qrImage->clear();
    m_status->setText("Starting a secure sign-in\u2026");
    spawnWorker("qr");
}

void SteamAuthDialog::refreshQr() {
    startQr();
}

void SteamAuthDialog::startPassword() {
    spawnWorker("password", m_pwUser->text().trimmed(), m_pwPass->text());
}

void SteamAuthDialog::spawnWorker(const QString& method, const QString& username,
                                   const QString& password) {
    cancelWorker();
    setCodeEntryEnabled(false);
    m_codeHint->clear();
    auto* worker = new SteamAuthWorker(method, username, password, this);
    connect(worker, &SteamAuthWorker::status, m_status, &QLabel::setText);
    connect(worker, &SteamAuthWorker::qrReady, this, &SteamAuthDialog::onQrReady);
    connect(worker, &SteamAuthWorker::codeRequested, this, &SteamAuthDialog::onCodeRequested);
    connect(worker, &SteamAuthWorker::codeRejected, this, &SteamAuthDialog::onCodeRejected);
    connect(worker, &SteamAuthWorker::done, this, &SteamAuthDialog::onDone);
    m_worker = worker;
    worker->start();
}

void SteamAuthDialog::cancelWorker() {
    if (m_worker && m_worker->isRunning()) {
        m_worker->cancel();
        m_worker->wait(8000);
    }
    m_worker = nullptr;
}

void SteamAuthDialog::onQrReady(const QString& url) {
    m_qrImage->setPixmap(qrPixmap(url));
    m_status->setText("Scan the code with the Steam mobile app to approve the sign-in.");
}

void SteamAuthDialog::onCodeRequested(int codeType, const QString& message) {
    Q_UNUSED(codeType);
    m_codeHint->setText(message);
    setCodeEntryEnabled(true);
    m_codeEdit->setFocus();
}

void SteamAuthDialog::onCodeRejected(const QString& message) {
    m_codeHint->setText(message);
    m_codeEdit->clear();
    setCodeEntryEnabled(true);
    m_codeEdit->setFocus();
}

void SteamAuthDialog::submitCode() {
    QString code = m_codeEdit->text().trimmed();
    if (code.isEmpty() || !m_worker) return;
    setCodeEntryEnabled(false);
    m_status->setText("Submitting your code\u2026");
    m_worker->submitCode(code);
}

void SteamAuthDialog::setCodeEntryEnabled(bool enabled) {
    m_codeEdit->setEnabled(enabled);
    m_codeBtn->setEnabled(enabled);
}

void SteamAuthDialog::onDone(bool ok, const QString& message) {
    if (!ok) {
        setCodeEntryEnabled(false);
        m_status->setText(message);
        m_status->setStyleSheet(QStringLiteral("color: %1; font-size: 11px; background: transparent;").arg(RED));
        return;
    }
    m_status->setStyleSheet(QStringLiteral("color: %1; font-size: 11px; background: transparent;").arg(ACCENT_1));
    m_status->setText(QStringLiteral("Signed in as %1.").arg(message));
    emit loginSucceeded(message);
    accept();
}

QPixmap SteamAuthDialog::qrPixmap(const QString& url) {
    Q_UNUSED(url);
    QPixmap pix(QR_PIXEL_SIZE, QR_PIXEL_SIZE);
    pix.fill(Qt::white);
    QPainter p(&pix);
    p.setPen(Qt::black);
    p.setFont(QFont("monospace", 10));
    p.drawText(pix.rect(), Qt::AlignCenter, "QR Code");
    return pix;
}

void SteamAuthDialog::cancel() {
    cancelWorker();
    reject();
}

void SteamAuthDialog::closeEvent(QCloseEvent* event) {
    cancelWorker();
    QDialog::closeEvent(event);
}
