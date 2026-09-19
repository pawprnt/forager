#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QButtonGroup>
#include <QStackedWidget>
#include <QFrame>
#include <QWebEngineView>
#include <QUrl>
#include <QTimer>

class WebStorePane : public QWidget {
    Q_OBJECT

public:
    explicit WebStorePane(const QUrl& url, QWidget* parent = nullptr);
    void load();

private slots:
    void onLoadFinished(bool ok);

private:
    void retryCss();

    QWebEngineView* m_view = nullptr;
    QUrl m_url;
    bool m_cssInjected = false;
    int m_cssRetries = 0;
    static constexpr int CSS_MAX_RETRIES = 15;
};

class StorePage : public QWidget {
    Q_OBJECT

public:
    explicit StorePage(QWidget* parent = nullptr);

private slots:
    void switchTab(QAbstractButton* button);

private:
    QWidget* buildTabs();
    QWidget* buildPlaceholder(const QString& name);

    QStackedWidget* m_stack = nullptr;
    WebStorePane* m_webPane = nullptr;
    QButtonGroup* m_tabsGroup = nullptr;
};
