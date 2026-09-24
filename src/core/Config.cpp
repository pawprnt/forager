#include "core/Config.h"
#include "core/Paths.h"
#include "app/Constants.h"
#include "utils/Json.h"

#include <QDir>
#include <QStandardPaths>

Config& Config::instance()
{
    static Config s_instance;
    return s_instance;
}

static QJsonObject defaultConfig()
{
    QJsonObject o;
    o["games_dir"] = QDir::homePath() + "/Games";
    o["steam_appcache"] = QDir::homePath()
        + "/.local/share/Steam/appcache/librarycache";
    o["display_size"] = "medium";

    QJsonObject proton;
    QJsonObject features;
    features["rpgmaker_vxace_rtp"] = false;
    proton["features"] = features;
    o["proton"] = proton;

    return o;
}

void Config::load()
{
    m_path = paths::configDir() + "/settings.json";
    auto data = json::readObject(m_path);
    m_data = deepMerge(defaultConfig(), data.value_or(QJsonObject{}));
}

void Config::save()
{
    json::writeObject(m_path, m_data);
}

QString Config::gamesDir() const
{
    return m_data["games_dir"].toString();
}

QString Config::steamAppcache() const
{
    return m_data["steam_appcache"].toString();
}

QString Config::displaySize() const
{
    return m_data["display_size"].toString();
}

bool Config::protonFeature(const QString& name) const
{
    return m_data["proton"].toObject()["features"].toObject()[name].toBool();
}

void Config::setGamesDir(const QString& dir)
{
    m_data["games_dir"] = dir;
}

void Config::setSteamAppcache(const QString& path)
{
    m_data["steam_appcache"] = path;
}

void Config::setDisplaySize(const QString& size)
{
    m_data["display_size"] = size;
}

QJsonObject Config::deepMerge(const QJsonObject& base, const QJsonObject& override_)
{
    QJsonObject result = base;
    for (auto it = override_.begin(); it != override_.end(); ++it) {
        if (it.value().isObject() && result[it.key()].isObject()) {
            result[it.key()] = deepMerge(result[it.key()].toObject(), it.value().toObject());
        } else {
            result[it.key()] = it.value();
        }
    }
    return result;
}
