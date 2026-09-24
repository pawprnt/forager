#include "minecraft/Assets.h"
#include "utils/Filesystem.h"
#include "utils/Json.h"
#include "utils/Network.h"

#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>
#include <QJsonObject>

static QString fileSha1Hex(const QString& path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return {};
    return QString::fromLatin1(
        QCryptographicHash::hash(f.readAll(), QCryptographicHash::Sha1).toHex());
}

static bool fileMatchesSize(const QString& path, qint64 size) {
    const QFileInfo info(path);
    if (!info.isFile()) return false;
    return size <= 0 || info.size() == size;
}

static bool copyFile(const QString& src, const QString& dst) {
    if (!fs::ensureParentDir(dst)) return false;
    return QFile::copy(src, dst);
}

namespace mc {

AssetsResult ensureAssets(const VersionJson& version, const QString& assetsRoot,
                          const QString& librariesRoot) {
    Q_UNUSED(librariesRoot);

    AssetsResult result;
    result.assetsDir = assetsRoot;

    QString indexId;
    QString indexUrl;
    QString indexSha1;
    if (!version.assetIndex.isEmpty()) {
        indexId = version.assetIndex.value(QStringLiteral("id")).toString();
        if (indexId.isEmpty()) indexId = version.assets;
        indexUrl = version.assetIndex.value(QStringLiteral("url")).toString();
        indexSha1 = version.assetIndex.value(QStringLiteral("sha1")).toString();
    } else {
        indexId = version.assets;
    }

    if (indexId.isEmpty()) {
        result.error = QStringLiteral("no asset index");
        return result;
    }
    result.indexId = indexId;

    const QString indexPath = assetsRoot + "/indexes/" + indexId + ".json";
    bool indexReady = QFile::exists(indexPath);
    if (indexReady && !indexSha1.isEmpty())
        indexReady = fileSha1Hex(indexPath).compare(indexSha1, Qt::CaseInsensitive) == 0;

    if (!indexReady) {
        if (indexUrl.isEmpty()) {
            result.error = QStringLiteral("asset index missing");
            return result;
        }
        const QByteArray data = net::httpGet(indexUrl);
        if (data.isEmpty()) {
            result.error = QStringLiteral("asset index download failed");
            return result;
        }
        if (!fs::writeBytes(indexPath, data)) {
            result.error = QStringLiteral("asset index write failed");
            return result;
        }
    }

    const auto index = json::readObject(indexPath);
    if (!index) {
        result.error = QStringLiteral("asset index invalid");
        return result;
    }

    const QJsonObject objects = index->value(QStringLiteral("objects")).toObject();
    const bool mirror = index->value(QStringLiteral("map_to_resources")).toBool() ||
                        index->value(QStringLiteral("virtual")).toBool();

    int failed = 0;
    for (auto it = objects.begin(); it != objects.end(); ++it) {
        const QJsonObject entry = it.value().toObject();
        const QString hash = entry.value(QStringLiteral("hash")).toString();
        if (hash.isEmpty()) continue;
        const qint64 size = static_cast<qint64>(entry.value(QStringLiteral("size")).toDouble());
        const QString rel = hash.left(2) + "/" + hash;
        const QString objectPath = assetsRoot + "/objects/" + rel;

        if (!fileMatchesSize(objectPath, size)) {
            const QByteArray data =
                net::httpGet(QStringLiteral("https://resources.download.minecraft.net/") + rel);
            if (data.isEmpty() || (size > 0 && data.size() != size) ||
                !fs::writeBytes(objectPath, data)) {
                ++failed;
                continue;
            }
        }

        if (mirror) {
            const QString virtualPath = assetsRoot + "/virtual/" + indexId + "/" + it.key();
            if (!QFile::exists(virtualPath) && !copyFile(objectPath, virtualPath)) ++failed;
        }
    }

    if (failed > 0) {
        result.error = QStringLiteral("%1 object(s) failed").arg(failed);
        return result;
    }

    result.ok = true;
    return result;
}

bool ensureClientJar(const VersionJson& version, const QString& librariesRoot,
                     QString* clientJarPath, QString* error) {
    if (clientJarPath) clientJarPath->clear();
    if (error) error->clear();

    QString jarPath;
    const QJsonObject client = version.downloads.value(QStringLiteral("client")).toObject();
    if (!client.isEmpty()) {
        QString rel = client.value(QStringLiteral("path")).toString();
        if (rel.isEmpty())
            rel = QStringLiteral("com/mojang/minecraft/%1/minecraft-%1-client.jar")
                      .arg(version.id);
        jarPath = librariesRoot + "/" + rel;
        const qint64 size = static_cast<qint64>(client.value(QStringLiteral("size")).toDouble());
        if (fileMatchesSize(jarPath, size)) {
            if (clientJarPath) *clientJarPath = jarPath;
            return true;
        }
        const QString url = client.value(QStringLiteral("url")).toString();
        if (url.isEmpty()) {
            if (error) *error = QStringLiteral("client jar missing and no download URL");
            return false;
        }
        const QByteArray data = net::httpGet(url);
        if (data.isEmpty() || (size > 0 && data.size() != size) ||
            !fs::writeBytes(jarPath, data)) {
            if (error) *error = QStringLiteral("client jar download failed");
            return false;
        }
        if (clientJarPath) *clientJarPath = jarPath;
        return true;
    }

    jarPath = librariesRoot + "/com/mojang/minecraft/" + version.id +
              "/minecraft-" + version.id + "-client.jar";
    if (QFile::exists(jarPath)) {
        if (clientJarPath) *clientJarPath = jarPath;
        return true;
    }
    if (error) *error = QStringLiteral("client jar missing and no download URL");
    return false;
}

} // namespace mc
