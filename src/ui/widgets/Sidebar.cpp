#include "ui/widgets/Sidebar.h"

#include <QVBoxLayout>
#include <QLineEdit>

Sidebar::Sidebar(QWidget* parent)
    : QWidget(parent)
{
    setFixedWidth(240);
    setStyleSheet("background-color: #13151a;");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);

    m_search = new QLineEdit(this);
    m_search->setPlaceholderText("Search...");
    m_search->setStyleSheet(R"(
        QLineEdit {
            background-color: #1e2028;
            border: 1px solid #2a2d38;
            border-radius: 6px;
            padding: 8px 12px;
            color: #e5e7eb;
            font-size: 13px;
        }
        QLineEdit:focus {
            border-color: #6366f1;
        }
    )");
    layout->addWidget(m_search);

    // TODO: game list, download box, user panel
    layout->addStretch();
}
