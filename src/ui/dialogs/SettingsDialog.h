#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStackedWidget>
#include <QButtonGroup>
#include <QDialogButtonBox>
#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QSize>

class QLineEdit;

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);

    QString selectedCardSize() const;
    QString gamesDirText() const;
    QString steamAppcacheText() const;

signals:
    void updateProtonRequested();
    void gamesDirChanged();

private slots:
    void save();

private:
    QWidget* buildHeader();
    QWidget* buildNav();
    QWidget* buildFooter();
    QPushButton* navButton(const QString& text, const QString& icon);
    void switchTab(int index);

    QStackedWidget* m_pages = nullptr;

    QWidget* m_libraryTab = nullptr;
    QWidget* m_protonTab = nullptr;
    QWidget* m_accountTab = nullptr;

    QLineEdit* m_dirEdit = nullptr;
    QLineEdit* m_cacheEdit = nullptr;
    QButtonGroup* m_navGroup = nullptr;
};
