#pragma once
#include <QString>
#include <QByteArray>
#include <QMap>
#include <functional>

namespace net {
    using Headers = QMap<QString, QString>;
    Headers bearerHeaders(const QString& token);
    QByteArray httpGet(const QString& url, int timeoutMs = 15000,
                       const Headers& headers = Headers());
    QByteArray httpPost(const QString& url, const QByteArray& body,
                        const Headers& headers = Headers(), int timeoutMs = 15000);
} // namespace net
