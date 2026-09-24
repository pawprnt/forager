#include "ui/dialogs/SteamAuthDialog.h"
#include "ui/Theme.h"
#include "ui/Style.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QUrlQuery>
#include <QCloseEvent>
#include <QMutexLocker>
#include <QPainter>
#include <QEventLoop>
#include <QTimer>

using namespace theme;
using namespace theme::C;

static const int QR_PIXEL_SIZE = 256;

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

QByteArray SteamAuthWorker::waitForReply(QNetworkReply* reply) {
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QTimer::singleShot(15000, &loop, &QEventLoop::quit);
    loop.exec();
    QByteArray data = reply->readAll();
    reply->deleteLater();
    return data;
}

QJsonObject SteamAuthWorker::postForm(QNetworkAccessManager& nam, const QUrl& url, const QUrlQuery& params) {
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply* reply = nam.post(req, params.toString(QUrl::FullyEncoded).toUtf8());
    return QJsonDocument::fromJson(waitForReply(reply)).object();
}

QJsonObject SteamAuthWorker::postJson(QNetworkAccessManager& nam, const QUrl& url, const QJsonObject& body) {
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply* reply = nam.post(req, QJsonDocument(body).toJson());
    return QJsonDocument::fromJson(waitForReply(reply)).object();
}

bool SteamAuthWorker::isCancelled() {
    QMutexLocker lock(&m_mutex);
    return m_cancelled;
}

QString SteamAuthWorker::drainCode() {
    QMutexLocker lock(&m_mutex);
    QString code = m_pendingCode;
    m_pendingCode.clear();
    return code;
}

void SteamAuthWorker::run() {
    QNetworkAccessManager nam;

    try {
        QString clientId;
        QString requestId;
        int interval = 5;

        if (m_method == "qr") {
            if (!beginQrSession(nam, clientId, requestId, interval)) return;
        } else {
            if (!beginPasswordSession(nam, clientId, requestId, interval)) return;
        }

        while (!isCancelled()) {
            QString code = drainCode();
            if (!code.isEmpty()) {
                QUrlQuery params;
                params.addQueryItem("client_id", clientId);
                params.addQueryItem("steam_code", code);
                postForm(nam, QUrl("https://api.steampowered.com/IAuthenticationService/UpdateAuthSessionWithSteamGuardCode/v1/"), params);
                emit status("Checking your code\u2026");
            }

            QUrlQuery pollParams;
            pollParams.addQueryItem("client_id", clientId);
            pollParams.addQueryItem("request_id", requestId);
            QJsonObject pollResp = postForm(
                nam, QUrl("https://api.steampowered.com/IAuthenticationService/PollAuthSessionStatus/v1/"), pollParams);
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
    style::background(this, BG);

    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(20, 20, 20, 20);
    lay->setSpacing(10);

    m_title = style::heading("Sign in with Steam", 18);
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
    style::label(m_qrHint, TEXT_DIM, 11);
    m_qrBox->addWidget(m_qrHint);
    m_qrRefreshBtn = style::button("Refresh QR code", "secondary");
    connect(m_qrRefreshBtn, &QPushButton::clicked, this, &SteamAuthDialog::refreshQr);
    m_qrBox->addWidget(m_qrRefreshBtn);
    m_qrPasswordLink = style::button("Use password instead", "ghost");
    connect(m_qrPasswordLink, &QPushButton::clicked, this, [this] { showMode(false); });
    m_qrBox->addWidget(m_qrPasswordLink);
    lay->addLayout(m_qrBox);

    // Password mode
    m_pwBox = new QVBoxLayout();
    m_pwBox->setSpacing(8);
    m_pwUser = new QLineEdit();
    m_pwUser->setPlaceholderText("Steam account name");
    m_pwUser->setStyleSheet(style::lineeditQss());
    m_pwBox->addWidget(m_pwUser);
    m_pwPass = new QLineEdit();
    m_pwPass->setPlaceholderText("Password");
    m_pwPass->setEchoMode(QLineEdit::Password);
    m_pwPass->setStyleSheet(style::lineeditQss());
    m_pwBox->addWidget(m_pwPass);
    m_pwSignInBtn = style::button("Sign in", "primary");
    connect(m_pwSignInBtn, &QPushButton::clicked, this, &SteamAuthDialog::startPassword);
    m_pwBox->addWidget(m_pwSignInBtn);
    m_pwQrLink = style::button("Use QR instead", "ghost");
    connect(m_pwQrLink, &QPushButton::clicked, this, [this] { showMode(true); });
    m_pwBox->addWidget(m_pwQrLink);
    lay->addLayout(m_pwBox);

    // Steam Guard code row
    m_codeRow = new QHBoxLayout();
    m_codeRow->setSpacing(8);
    m_codeEdit = new QLineEdit();
    m_codeEdit->setPlaceholderText("Steam Guard code");
    m_codeEdit->setStyleSheet(style::lineeditQss());
    connect(m_codeEdit, &QLineEdit::returnPressed, this, &SteamAuthDialog::submitCode);
    m_codeEdit->setEnabled(false);
    m_codeRow->addWidget(m_codeEdit, 1);
    m_codeBtn = style::button("Submit", "primary");
    m_codeBtn->setEnabled(false);
    connect(m_codeBtn, &QPushButton::clicked, this, &SteamAuthDialog::submitCode);
    m_codeRow->addWidget(m_codeBtn);
    lay->addLayout(m_codeRow);

    m_codeHint = new QLabel();
    m_codeHint->setWordWrap(true);
    style::label(m_codeHint, TEXT_DIM, 11);
    lay->addWidget(m_codeHint);

    m_status = new QLabel();
    m_status->setWordWrap(true);
    style::label(m_status, TEXT_DIM, 11);
    lay->addWidget(m_status);
    lay->addStretch(1);

    showMode(true);
    startQr();
}

void SteamAuthDialog::showMode(bool qr) {
    m_pwUser->setVisible(!qr);
    m_pwPass->setVisible(!qr);
    m_pwSignInBtn->setVisible(!qr);
    m_pwQrLink->setVisible(!qr);
    m_qrImage->setVisible(qr);
    m_qrHint->setVisible(qr);
    m_qrRefreshBtn->setVisible(qr);
    m_qrPasswordLink->setVisible(qr);
    if (!qr)
        m_pwUser->setFocus();
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
        style::label(m_status, RED, 11);
        return;
    }
    style::label(m_status, ACCENT_1, 11);
    m_status->setText(QStringLiteral("Signed in as %1.").arg(message));
    emit loginSucceeded(message);
    accept();
}

void SteamAuthDialog::cancel() {
    cancelWorker();
    reject();
}

void SteamAuthDialog::closeEvent(QCloseEvent* event) {
    cancelWorker();
    QDialog::closeEvent(event);
}
