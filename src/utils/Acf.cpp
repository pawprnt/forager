#include "utils/Acf.h"
#include <QDir>
#include <QFile>
#include <QRegularExpression>

static QString acfValue(const QString& text, const QString& key)
{
    QRegularExpression re(QStringLiteral("\"%1\"\\s+\"(.+?)\"").arg(key));
    auto m = re.match(text);
    return m.hasMatch() ? m.captured(1) : QString();
}

std::pair<QString, QString> acf::parseFile(const QString& filePath)
{
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};

    QString text = QString::fromUtf8(f.readAll());
    QString appId = acfValue(text, "appid");
    QString name = acfValue(text, "name");
    name.remove(QChar::Null);
    return {appId, name};
}

QStringList acf::listManifests(const QString& steamappsDir)
{
    QDir dir(steamappsDir);
    if (!dir.exists()) return {};

    QStringList out;
    for (const auto& name : dir.entryList({"appmanifest_*.acf"}, QDir::Files, QDir::Name))
        out.append(dir.absoluteFilePath(name));
    return out;
}
