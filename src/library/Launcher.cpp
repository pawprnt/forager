#include "library/Launcher.h"

#include <QDir>
#include <QFileInfo>
#include <QProcess>

static std::unique_ptr<QProcess> launchSteam(const Game& game)
{
    auto proc = std::make_unique<QProcess>();
    proc->setProgram("steam");
    proc->setArguments({QString("steam://rungameid/%1").arg(game.appId())});
    proc->setProcessChannelMode(QProcess::ForwardedChannels);
    proc->start();
    return proc;
}

static std::unique_ptr<QProcess> launchMinecraft(const Game& game)
{
    auto proc = std::make_unique<QProcess>();
    proc->setProgram("prismlauncher");
    proc->setArguments({"-l", game.name()});
    proc->setProcessChannelMode(QProcess::ForwardedChannels);
    proc->start();
    return proc;
}

static QString findExecutable(const QString& dirPath)
{
    QDir dir(dirPath);
    if (!dir.exists()) return {};

    // Check if the path itself is an executable file
    QFileInfo info(dirPath);
    if (info.isFile() && info.isExecutable()) return dirPath;

    // Search for executables
    for (const auto& pattern : {"*.x86_64", "*.sh", "*.py", "*.exe"}) {
        QStringList files = dir.entryList({pattern}, QDir::Files, QDir::Name);
        if (!files.isEmpty()) return dir.absoluteFilePath(files.first());
    }
    return {};
}

static std::unique_ptr<QProcess> launchStandalone(const Game& game)
{
    if (game.path().isEmpty()) return nullptr;

    QString exe = findExecutable(game.path());
    if (exe.isEmpty()) return nullptr;

    auto proc = std::make_unique<QProcess>();
    proc->setWorkingDirectory(game.path());

    if (exe.endsWith(".exe")) {
        // TODO: launch via proton
        return nullptr;
    }

    proc->setProgram(exe);
    proc->setProcessChannelMode(QProcess::ForwardedChannels);
    proc->start();
    return proc;
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
