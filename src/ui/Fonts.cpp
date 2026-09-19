#include "ui/Fonts.h"
#include <QFontDatabase>
#include <QCoreApplication>
#include <QFileInfo>

static bool s_registered = false;

void fonts::registerFonts() {
    if (s_registered) return;

    QString fontsDir = QCoreApplication::applicationDirPath() + "/../share/forager/fonts";
    QStringList weights = {
        "BeVietnamPro-Light.ttf",
        "BeVietnamPro-Regular.ttf",
        "BeVietnamPro-Medium.ttf",
        "BeVietnamPro-SemiBold.ttf",
        "BeVietnamPro-Bold.ttf",
        "BeVietnamPro-ExtraBold.ttf",
    };

    for (const auto& w : weights) {
        QString path = fontsDir + "/" + w;
        if (QFileInfo::exists(path)) {
            QFontDatabase::addApplicationFont(path);
        }
    }
    s_registered = true;
}
