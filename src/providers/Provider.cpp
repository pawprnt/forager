#include "providers/Provider.h"

void Provider::requireConfigured(const char* msg) const
{
    if (!isConfigured()) {
        throw BackendNotConfigured(msg);
    }
}

ProviderRegistry& ProviderRegistry::instance()
{
    static ProviderRegistry s_instance;
    return s_instance;
}

void ProviderRegistry::registerProvider(const QString& name, Factory factory)
{
    m_factories.emplace_back(name, std::move(factory));
}

std::unique_ptr<Provider> ProviderRegistry::create(const QString& name) const
{
    for (const auto& [n, factory] : m_factories) {
        if (n == name) return factory();
    }
    throw ProviderError("Unknown provider: " + name.toStdString());
}

QStringList ProviderRegistry::availableProviders() const
{
    QStringList names;
    for (const auto& [name, _] : m_factories) {
        names.append(name);
    }
    names.sort();
    return names;
}
