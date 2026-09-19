#pragma once

#include "providers/Provider.h"

class TorrentProvider : public Provider {
public:
    static constexpr const char* providerName() { return "torrent"; }

    TorrentProvider();

    QString name() const override { return QStringLiteral("torrent"); }
    bool isConfigured() const override;
    std::vector<OwnedGame> listOwned(const QString& account = {}) const override;
    void download(const QString& appId, const QString& destination,
                  ProgressFn onProgress = nullptr,
                  std::atomic<bool>* cancel = nullptr) override;

private:
    bool isLibtorrentAvailable() const;
};
