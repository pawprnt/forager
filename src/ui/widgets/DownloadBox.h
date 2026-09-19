#pragma once

#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

class ProgressBar;

class DownloadBox : public QFrame {
    Q_OBJECT

public:
    explicit DownloadBox(QWidget* parent = nullptr);

    void begin(const QString& name);
    void setProgress(double percent, const QString& stage, double speed, double done, double total);
    void hideDownload();

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent* event) override;

private:
    QLabel* m_name = nullptr;
    QLabel* m_percent = nullptr;
    ProgressBar* m_bar = nullptr;
    QLabel* m_detail = nullptr;
};
