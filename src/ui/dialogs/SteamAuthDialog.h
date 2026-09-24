#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QThread>
#include <QPixmap>
#include <QMutex>
#include <QWaitCondition>
#include <QJsonObject>
#include <QUrl>
#include <functional>

class QNetworkAccessManager;
class QNetworkReply;
class QUrlQuery;

class SteamAuthWorker : public QThread {
    Q_OBJECT

public:
    explicit SteamAuthWorker(const QString& method, const QString& username = {},
                             const QString& password = {}, QObject* parent = nullptr);

    void submitCode(const QString& code);
    void cancel();

signals:
    void status(const QString& text);
    void qrReady(const QString& url);
    void codeRequested(int codeType, const QString& message);
    void codeRejected(const QString& message);
    void done(bool ok, const QString& message);

protected:
    void run() override;

private:
    QByteArray waitForReply(QNetworkReply* reply);
    QJsonObject postForm(QNetworkAccessManager& nam, const QUrl& url, const QUrlQuery& params);
    QJsonObject postJson(QNetworkAccessManager& nam, const QUrl& url, const QJsonObject& body);
    bool isCancelled();
    QString drainCode();
    bool beginQrSession(QNetworkAccessManager& nam, QString& clientId, QString& requestId, int& interval);
    bool beginPasswordSession(QNetworkAccessManager& nam, QString& clientId, QString& requestId, int& interval);

    QString m_method;
    QString m_username;
    QString m_password;
    QMutex m_mutex;
    QWaitCondition m_codeReady;
    QString m_pendingCode;
    bool m_cancelled = false;
    bool m_doneEmitted = false;
};

class SteamAuthDialog : public QDialog {
    Q_OBJECT

public:
    explicit SteamAuthDialog(QWidget* parent = nullptr);

    void cancel();

signals:
    void loginSucceeded(const QString& accountName);

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void showMode(bool qr);
    void startQr();
    void startPassword();
    void refreshQr();
    void submitCode();
    void onQrReady(const QString& url);
    void onCodeRequested(int codeType, const QString& message);
    void onCodeRejected(const QString& message);
    void onDone(bool ok, const QString& message);

private:
    void spawnWorker(const QString& method, const QString& username = {},
                     const QString& password = {});
    void cancelWorker();
    void setCodeEntryEnabled(bool enabled);
    QPixmap qrPixmap(const QString& url);

    SteamAuthWorker* m_worker = nullptr;

    QVBoxLayout* m_qrBox = nullptr;
    QLabel* m_qrImage = nullptr;
    QLabel* m_qrHint = nullptr;
    QPushButton* m_qrRefreshBtn = nullptr;
    QPushButton* m_qrPasswordLink = nullptr;

    QVBoxLayout* m_pwBox = nullptr;
    QLineEdit* m_pwUser = nullptr;
    QLineEdit* m_pwPass = nullptr;
    QPushButton* m_pwSignInBtn = nullptr;
    QPushButton* m_pwQrLink = nullptr;

    QHBoxLayout* m_codeRow = nullptr;
    QLineEdit* m_codeEdit = nullptr;
    QPushButton* m_codeBtn = nullptr;
    QLabel* m_codeHint = nullptr;
    QLabel* m_status = nullptr;
    QLabel* m_title = nullptr;
};
