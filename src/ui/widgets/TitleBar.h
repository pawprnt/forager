#pragma once

#include <QWidget>

class QToolButton;
class QPushButton;
class QLabel;
class QButtonGroup;
class QMenu;

class TitleBar : public QWidget {
    Q_OBJECT

public:
    explicit TitleBar(QWidget* parent = nullptr);

    void setActiveTab(const QString& name);
    void setBackEnabled(bool enabled);
    void setControllerHint(const QString& text);
    void setUpdates(int count);

signals:
    void settingsRequested();
    void updateProtonRequested();
    void runUpdatesRequested();
    void backRequested();
    void storeTabRequested();
    void libraryTabRequested();

private:
    QPushButton* navButton(const QString& iconName);

    QToolButton* _logo;
    QPushButton* _backBtn;
    QPushButton* _forwardBtn;
    QPushButton* _storeTab;
    QPushButton* _libraryTab;
    QButtonGroup* _tabsGroup;
    QPushButton* _updatePill;
    QLabel* _controllerHint;
    QMenu* _mainMenu;
};
