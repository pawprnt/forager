#pragma once

#include <QWidget>
#include <QPixmap>
#include <QPainterPath>

class Banner : public QWidget {
    Q_OBJECT

public:
    explicit Banner(QWidget* parent = nullptr);

    void setSource(const QPixmap& pix, bool fit = false);
    void setOverlay(QWidget* overlay);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    QPixmap backdrop(const QPixmap& pix);
    QPixmap crisp(const QPixmap& pix);
    QPainterPath clipPath() const;

    QPixmap m_source;
    bool m_fit = false;
    QWidget* m_overlay = nullptr;

    std::tuple<qint64, int, int> m_backdropCacheKey;
    QPixmap m_backdropCache;
    std::tuple<qint64, int, int> m_crispCacheKey;
    QPixmap m_crispCache;

    static constexpr int BANNER_H = 420;
    static constexpr double BLUR_RADIUS = 26;
    static constexpr int FEATHER = 130;
};
