#include "ui/dialogs/SteamAuthDialog.h"
#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QJsonObject>

bool SteamAuthWorker::beginPasswordSession(QNetworkAccessManager& nam, QString& clientId,
                                           QString& requestId, int& interval) {
    if (m_username.isEmpty() || m_password.isEmpty()) {
        emit done(false, "Enter your Steam account name and password.");
        return false;
    }
    QJsonObject body;
    body["account_name"] = m_username;
    QJsonObject resp = postJson(
        nam, QUrl("https://api.steampowered.com/IAuthenticationService/BeginAuthSessionViaCredentials/v1/"), body);
    QJsonObject r = resp.value("response").toObject();
    clientId = QString::number(static_cast<qint64>(r.value("client_id").toDouble()));
    requestId = QString::number(static_cast<qint64>(r.value("request_id").toDouble()));
    interval = r.value("interval").toInt(5);
    emit status("Enter your credentials.");
    return true;
}

void SteamAuthDialog::startPassword() {
    spawnWorker("password", m_pwUser->text().trimmed(), m_pwPass->text());
}
