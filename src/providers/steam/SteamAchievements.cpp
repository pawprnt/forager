#include "providers/steam/SteamAchievements.h"
#include "core/Paths.h"

#include <QFile>
#include <QDir>
#include <QTextStream>

QVector<Achievement> SteamAchievements::readFromFile(const QString& steamId,
                                                     const QString& appId) const
{
    QString path = paths::steamRoot() +
        "/userdata/" + steamId +
        "/" + appId + "/achievements.vdf";

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    QByteArray data = file.readAll();
    VdfEntry root = parseVdf(data);
    return extractAchievements(root);
}

static bool isWhitespace(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }

static QString parseQuoted(const QByteArray& data, int& pos)
{
    if (pos >= data.size() || data[pos] != '"') return {};
    pos++;
    int start = pos;
    while (pos < data.size() && data[pos] != '"') {
        if (data[pos] == '\\') pos++;
        pos++;
    }
    QString result = QString::fromUtf8(data.mid(start, pos - start));
    if (pos < data.size()) pos++;
    return result;
}

static void skipWhitespace(const QByteArray& data, int& pos)
{
    while (pos < data.size() && isWhitespace(data[pos])) pos++;
}

VdfEntry SteamAchievements::parseVdf(const QByteArray& data)
{
    VdfEntry root;
    root.key = "root";
    QVector<VdfEntry*> stack;
    stack.push_back(&root);

    int pos = 0;
    int size = data.size();

    while (pos < size) {
        skipWhitespace(data, pos);
        if (pos >= size) break;

        if (data[pos] == '{') {
            pos++;
            continue;
        }

        if (data[pos] == '}') {
            pos++;
            if (stack.size() > 1) stack.pop_back();
            continue;
        }

        QString key = parseQuoted(data, pos);
        if (key.isEmpty()) { pos++; continue; }

        skipWhitespace(data, pos);
        if (pos >= size) break;

        if (data[pos] == '"') {
            VdfEntry entry;
            entry.key = key;
            entry.value = parseQuoted(data, pos);
            stack.last()->children.push_back(std::move(entry));
        } else if (data[pos] == '{') {
            pos++;
            VdfEntry entry;
            entry.key = key;
            stack.last()->children.push_back(std::move(entry));
            stack.push_back(&stack.last()->children.last());
        } else {
            pos++;
        }
    }

    return root;
}

static const VdfEntry* findChild(const VdfEntry& entry, const QString& key)
{
    for (const auto& child : entry.children) {
        if (child.key == key) return &child;
    }
    return nullptr;
}

QVector<Achievement> SteamAchievements::extractAchievements(const VdfEntry& root)
{
    QVector<Achievement> result;

    const VdfEntry* stats = findChild(root, "stats");
    if (!stats) return result;

    for (const auto& child : stats->children) {
        if (child.key != "achievement") continue;

        Achievement a;
        if (const VdfEntry* e = findChild(child, "name"))
            a.apiName = e->value;
        if (const VdfEntry* e = findChild(child, "display_name"))
            a.displayName = e->value;
        if (const VdfEntry* e = findChild(child, "description"))
            a.description = e->value;
        if (const VdfEntry* e = findChild(child, "achieved"))
            a.achieved = e->value == "1";
        if (const VdfEntry* e = findChild(child, "unlock_time"))
            a.unlockTime = e->value.toLongLong();

        if (!a.apiName.isEmpty()) {
            result.push_back(std::move(a));
        }
    }

    return result;
}
