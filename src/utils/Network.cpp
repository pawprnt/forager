#include "utils/Network.h"
#include "app/Constants.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>
#include <QUrl>

static QNetworkAccessManager& manager() {
    static QNetworkAccessManager m;
    return m;
}

QByteArray net::httpGet(const QString& url, int timeoutMs, const Headers& headers) {
    QUrl qurl(url);
    QNetworkRequest req(qurl);
    req.setHeader(QNetworkRequest::UserAgentHeader,
                  QStringLiteral("forager/%1").arg(constants::APP_VERSION));
    for (auto it = headers.begin(); it != headers.end(); ++it)
        req.setRawHeader(it.key().toUtf8(), it.value().toUtf8());

    QNetworkReply* reply = manager().get(req);
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QTimer::singleShot(timeoutMs, &loop, &QEventLoop::quit);
    loop.exec();

    QByteArray data = reply->readAll();
    reply->deleteLater();
    return data;
}

QByteArray net::httpPost(const QString& url, const QByteArray& body,
                         const Headers& headers, int timeoutMs) {
    QUrl qurl(url);
    QNetworkRequest req(qurl);
    req.setHeader(QNetworkRequest::UserAgentHeader,
                  QStringLiteral("forager/%1").arg(constants::APP_VERSION));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    for (auto it = headers.begin(); it != headers.end(); ++it)
        req.setRawHeader(it.key().toUtf8(), it.value().toUtf8());

    QNetworkReply* reply = manager().post(req, body);
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QTimer::singleShot(timeoutMs, &loop, &QEventLoop::quit);
    loop.exec();

    QByteArray data = reply->readAll();
    reply->deleteLater();
    return data;
}
