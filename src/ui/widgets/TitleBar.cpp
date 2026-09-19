#include "ui/widgets/TitleBar.h"

#include <QHBoxLayout>
#include <QLabel>

TitleBar::TitleBar(QWidget* parent)
    : QWidget(parent)
{
    setFixedHeight(48);
    setStyleSheet("background-color: #0e1015; border-bottom: 1px solid #1e2028;");

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(16, 0, 16, 0);

    auto* logo = new QLabel("forager", this);
    logo->setStyleSheet("color: #e5e7eb; font-size: 16px; font-weight: bold; font-family: 'Be Vietnam Pro';");
    layout->addWidget(logo);

    layout->addStretch();

    // TODO: nav buttons, tabs, update pill, gamepad hint
}
