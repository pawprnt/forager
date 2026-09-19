#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QUrl>
#include <QWebEngineView>

class SteamGridDBTokenDialog : public QDialog {
    Q_OBJECT

public:
    explicit SteamGridDBTokenDialog(QWidget* parent = nullptr);

    void cancel();

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void poll();
    void readPoll();
    void onPollResult(const QVariant& result);
    void onLoadFinished(bool ok);

private:
    QWebEngineView* m_view = nullptr;
    QLabel* m_status = nullptr;
    QTimer* m_timer = nullptr;
    bool m_navigated = false;
    bool m_onApiPage = false;
};
