#include "library/Launcher.h"
#include "minecraft/Launch.h"
#include "minecraft/AuthSession.h"

#include <QDir>
#include <QFileInfo>
#include <QProcess>

static std::unique_ptr<QProcess> startProcess(const QString& program, const QStringList& args,
                                              const QString& cwd = {})
{
    auto proc = std::make_unique<QProcess>();
    if (!cwd.isEmpty()) proc->setWorkingDirectory(cwd);
    proc->setProgram(program);
    proc->setArguments(args);
    proc->setProcessChannelMode(QProcess::ForwardedChannels);
    proc->start();
    return proc;
}

static std::unique_ptr<QProcess> launchSteam(const Game& game)
{
    return startProcess("steam", {QString("steam://rungameid/%1").arg(game.appId())});
}

static std::unique_ptr<QProcess> launchMinecraft(const Game& game)
{
    if (game.path().isEmpty()) return nullptr;

    QString username = QString::fromUtf8(qgetenv("FORAGER_MC_USER"));
    if (username.isEmpty()) username = QStringLiteral("Player");
    mc::AuthSession session = mc::makeOfflineSession(username);
    if (session.playerName.isEmpty()) session = mc::makeOfflineSession(QStringLiteral("Player"));

    return mc::launchInstance(game.path(), session);
}

QString launcher::findExecutable(const QString& dirPath)
{
    QDir dir(dirPath);
    if (!dir.exists()) return {};

    QFileInfo info(dirPath);
    if (info.isFile() && info.isExecutable()) return dirPath;

    for (const auto& pattern : {"*.x86_64", "*.sh", "*.py", "*.exe"}) {
        QStringList files = dir.entryList({pattern}, QDir::Files, QDir::Name);
        if (!files.isEmpty()) return dir.absoluteFilePath(files.first());
    }
    return {};
}

static std::unique_ptr<QProcess> launchStandalone(const Game& game)
{
    if (game.path().isEmpty()) return nullptr;

    QString exe = launcher::findExecutable(game.path());
    if (exe.isEmpty()) return nullptr;

    if (exe.endsWith(".exe")) {
        // TODO: launch via proton
        return nullptr;
    }

    return startProcess(exe, {}, game.path());
}

std::unique_ptr<QProcess> launcher::launch(const Game& game)
{
    switch (game.source()) {
        case Source::Steam:      return launchSteam(game);
        case Source::Minecraft:  return launchMinecraft(game);
        case Source::Standalone: return launchStandalone(game);
    }
    return nullptr;
}
