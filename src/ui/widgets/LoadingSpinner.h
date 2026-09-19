#pragma once

#include <QWidget>
#include <QTimer>

class LoadingSpinner : public QWidget {
    Q_OBJECT

public:
    explicit LoadingSpinner(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;

private slots:
    void rotate();

private:
    int m_angle = 0;
    QTimer m_timer;
};
