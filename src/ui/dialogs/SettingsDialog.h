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
#include <QListWidget>
#include <QSize>

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);

    QString selectedCardSize() const;
    QString gamesDirText() const;

signals:
    void updateProtonRequested();
    void gamesDirChanged();

protected:
    void done(int result) override;

private slots:
    void save();

private:
    QWidget* buildHeader();
    QWidget* buildNav();
    QWidget* buildFooter();
    void switchTab(int index);

    QStackedWidget* m_pages = nullptr;
    QListWidget* m_navList = nullptr;

    QWidget* m_libraryTab = nullptr;
    QWidget* m_protonTab = nullptr;
    QWidget* m_accountTab = nullptr;
};
