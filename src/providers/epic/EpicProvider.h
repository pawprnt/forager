#pragma once

#include "providers/Provider.h"
#include <memory>

class EpicProvider : public Provider {
public:
    static constexpr const char* providerName() { return "epic"; }

    EpicProvider();

    QString name() const override { return QStringLiteral("epic"); }
    bool isConfigured() const override;
    std::vector<OwnedGame> listOwned(const QString& account = {}) const override;
    void download(const QString& appId, const QString& destination,
                  ProgressFn onProgress = nullptr,
                  std::atomic<bool>* cancel = nullptr) override;

private:
    bool isLegendaryInstalled() const;
    QString legendaryPath() const;
};
