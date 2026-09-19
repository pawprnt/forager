#pragma once

#include <QWidget>

class QLineEdit;

class Sidebar : public QWidget {
    Q_OBJECT

public:
    explicit Sidebar(QWidget* parent = nullptr);

private:
    QLineEdit* m_search = nullptr;
};
