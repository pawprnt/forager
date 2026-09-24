#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>

class ProgressBar : public QWidget {
    Q_OBJECT

public:
    explicit ProgressBar(int height = 4, QWidget* parent = nullptr);

    void setValue(double value);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    int m_height;
    double m_value = 0.0;
};

class DownloadsPage : public QWidget {
    Q_OBJECT

public:
    explicit DownloadsPage(QWidget* parent = nullptr);

    void setIdle();
    void begin(const QString& name);
    void setProgress(double percent, const QString& stage, double speed, double done, double total);
    void complete(const QString& version = QString());
    void failed(const QString& error);
    void cancelled();

signals:
    void cancelRequested();
    void settingsRequested();

private:
    class Banner;
    class StatItem;

    Banner* m_banner = nullptr;
    StatItem* m_speedStat = nullptr;
    StatItem* m_timeStat = nullptr;
    StatItem* m_spaceStat = nullptr;
    QFrame* m_item = nullptr;
    QLabel* m_itemName = nullptr;
    QLabel* m_itemStatus = nullptr;
    ProgressBar* m_itemBar = nullptr;
    QPushButton* m_itemCancel = nullptr;
    QLabel* m_empty = nullptr;

    void refreshSpace();
    void setStatus(const QString& status);
    void finish(const QString& status);
};
