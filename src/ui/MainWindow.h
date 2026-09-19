#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <QLabel>
#include <QLineEdit>
#include <QTimer>

class Sidebar;
class TitleBar;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void onSearchChanged(const QString& query);

private:
    void setupUi();
    void applyStyle();

    Sidebar* m_sidebar = nullptr;
    TitleBar* m_titlebar = nullptr;
    QStackedWidget* m_pages = nullptr;
    QLineEdit* m_search = nullptr;

    QTimer* m_startupTimer = nullptr;
};
