#include "ui/Fonts.h"
#include <QFontDatabase>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

static bool s_registered = false;

void fonts::registerFonts() {
    if (s_registered) return;

    QString installDir = QCoreApplication::applicationDirPath() + "/../share/forager/fonts";
    QString sourceDir = QCoreApplication::applicationDirPath() + "/../../src/forager/assets/fonts";

    QString fontsDir = QFileInfo::exists(installDir) ? installDir : sourceDir;

    QStringList files = {
        "BeVietnamPro-Light.ttf",
        "BeVietnamPro-Regular.ttf",
        "BeVietnamPro-Medium.ttf",
        "BeVietnamPro-SemiBold.ttf",
        "BeVietnamPro-Bold.ttf",
        "BeVietnamPro-ExtraBold.ttf",
        "VT323-Regular.ttf",
    };

    for (const auto& f : files) {
        QString path = fontsDir + "/" + f;
        if (QFileInfo::exists(path)) {
            QFontDatabase::addApplicationFont(path);
        }
    }
    s_registered = true;
}
