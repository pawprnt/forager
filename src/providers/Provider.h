#pragma once

#include <QString>
#include <QStringList>
#include <functional>
#include <memory>
#include <vector>
#include <stdexcept>

struct OwnedGame {
    QString app_id;
    QString name;
    QString provider;
    bool installed = false;
    QString icon_url;
};

struct DownloadProgress {
    QString name;
    double fraction = 0.0;
    qint64 bytes_received = 0;
    qint64 total_bytes = 0;
    int speed_bps = 0;
    int eta_seconds = -1;
};

using ProgressFn = std::function<void(const DownloadProgress&)>;

class ProviderError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class BackendNotConfigured : public ProviderError {
public:
    using ProviderError::ProviderError;
};

class Provider {
public:
    virtual ~Provider() = default;

    virtual QString name() const = 0;
    virtual bool isConfigured() const = 0;
    virtual std::vector<OwnedGame> listOwned(const QString& account = {}) const = 0;
    virtual void download(const QString& appId, const QString& destination,
                          ProgressFn onProgress = nullptr,
                          std::atomic<bool>* cancel = nullptr) = 0;
};

// Provider registry
class ProviderRegistry {
public:
    static ProviderRegistry& instance();

    using Factory = std::function<std::unique_ptr<Provider>()>;

    void registerProvider(const QString& name, Factory factory);
    std::unique_ptr<Provider> create(const QString& name) const;
    QStringList availableProviders() const;

private:
    ProviderRegistry() = default;
    std::vector<std::pair<QString, Factory>> m_factories;
};

// Helper to auto-register a provider
template <typename T>
struct AutoRegister {
    AutoRegister() {
        ProviderRegistry::instance().registerProvider(
            T::providerName(),
            []() { return std::make_unique<T>(); });
    }
};
