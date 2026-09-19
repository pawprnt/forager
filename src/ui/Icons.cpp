#include "ui/Icons.h"
#include <QSvgRenderer>
#include <QPixmap>
#include <QPainter>
#include <QMutex>
#include <QMutexLocker>
#include <QMap>
#include <QPair>
#include <QRegularExpression>
#include <QFileInfo>
#include <QCoreApplication>
#include <QFile>

static QMutex s_mutex;
static QMap<QPair<QString,QString>, QIcon> s_cache;

QIcon icons::loadIcon(const QString& name, const QString& color) {
    QString c = color.toLower();
    QPair<QString,QString> key(name, c);

    {
        QMutexLocker lock(&s_mutex);
        if (s_cache.contains(key)) return s_cache.value(key);
    }

    QString path = QCoreApplication::applicationDirPath()
        + "/../share/forager/icons/" + name + ".svg";
    if (!QFileInfo::exists(path)) return QIcon();

    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return QIcon();
    QByteArray svgData = f.readAll();
    f.close();

    QString svg = QString::fromUtf8(svgData);
    static QRegularExpression re(R"((stroke|fill)="currentColor")");
    svg.replace(re, QStringLiteral(R"(\1="%2")").arg(c));

    QSvgRenderer renderer;
    if (!renderer.load(svg.toUtf8())) return QIcon();

    QPixmap pixmap(renderer.defaultSize());
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    renderer.render(&painter);
    painter.end();

    QIcon icon(pixmap);
    {
        QMutexLocker lock(&s_mutex);
        s_cache.insert(key, icon);
    }
    return icon;
}
