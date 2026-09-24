#include "ui/dialogs/SteamAuthDialog.h"
#include "ui/Theme.h"
#include "ui/Style.h"
#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPainter>

using namespace theme;
using namespace theme::C;

static const int QR_PIXEL_SIZE = 256;

bool SteamAuthWorker::beginQrSession(QNetworkAccessManager& nam, QString& clientId,
                                     QString& requestId, int& interval) {
    QJsonObject dev;
    dev["device_name"] = "Forager";
    dev["device_type"] = 1;
    dev["os"] = "linux";
    QJsonObject body;
    body["device_details"] = dev;
    QJsonObject resp = postJson(
        nam, QUrl("https://api.steampowered.com/IAuthenticationService/BeginAuthSessionViaQR/v1/"), body);
    QJsonObject r = resp.value("response").toObject();
    QString challengeUrl = r.value("challenge_url").toString();
    clientId = QString::number(static_cast<qint64>(r.value("client_id").toDouble()));
    requestId = QString::number(static_cast<qint64>(r.value("request_id").toDouble()));
    interval = r.value("interval").toInt(5);
    emit qrReady(challengeUrl);
    emit status("Scan the QR code with the Steam mobile app.");
    return true;
}

void SteamAuthDialog::startQr() {
    m_qrImage->clear();
    m_status->setText("Starting a secure sign-in\u2026");
    spawnWorker("qr");
}

void SteamAuthDialog::refreshQr() {
    startQr();
}

void SteamAuthDialog::onQrReady(const QString& url) {
    m_qrImage->setPixmap(qrPixmap(url));
    m_status->setText("Scan the code with the Steam mobile app to approve the sign-in.");
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
