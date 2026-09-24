#include "utils/Network.h"
#include "app/Constants.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>
#include <QUrl>
#include <QSaveFile>

static thread_local QNetworkAccessManager* t_manager = nullptr;

static QNetworkAccessManager& manager() {
    if (!t_manager) t_manager = new QNetworkAccessManager();
    return *t_manager;
}

static QNetworkRequest buildRequest(const QString& url, const net::Headers& headers) {
    QNetworkRequest req{QUrl(url)};
    req.setHeader(QNetworkRequest::UserAgentHeader,
                  QStringLiteral("forager/%1").arg(constants::APP_VERSION));
    for (auto it = headers.begin(); it != headers.end(); ++it)
        req.setRawHeader(it.key().toUtf8(), it.value().toUtf8());
    return req;
}

static QByteArray awaitReply(QNetworkReply* reply, int timeoutMs) {
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QTimer::singleShot(timeoutMs, &loop, &QEventLoop::quit);
    loop.exec();

    QByteArray data;
    if (reply->error() == QNetworkReply::NoError) {
        int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (status >= 200 && status < 300)
            data = reply->readAll();
    }
    reply->deleteLater();
    return data;
}

net::Headers net::bearerHeaders(const QString& token) {
    if (token.isEmpty()) return {};
    return {{"Authorization", "Bearer " + token}};
}

QByteArray net::httpGet(const QString& url, int timeoutMs, const Headers& headers) {
    return awaitReply(manager().get(buildRequest(url, headers)), timeoutMs);
}

QByteArray net::httpPost(const QString& url, const QByteArray& body,
                         const Headers& headers, int timeoutMs) {
    QNetworkRequest req = buildRequest(url, headers);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    return awaitReply(manager().post(req, body), timeoutMs);
}
