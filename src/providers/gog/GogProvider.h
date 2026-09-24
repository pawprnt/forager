#pragma once

#include "providers/Provider.h"
#include "utils/Network.h"

#include <QJsonObject>

class GogProvider : public Provider {
public:
    static constexpr const char* providerName() { return "gog"; }

    GogProvider();

    QString name() const override { return QStringLiteral("gog"); }
    bool isConfigured() const override;
    std::vector<OwnedGame> listOwned(const QString& account = {}) const override;
    void download(const QString& appId, const QString& destination,
                  ProgressFn onProgress = nullptr,
                  std::atomic<bool>* cancel = nullptr) override;

    void setBearerToken(const QString& token);

private:
    QJsonObject getFilteredProducts() const;
    QString m_bearerToken;
};
