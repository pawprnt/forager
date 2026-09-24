#include "ui/MainWindow.h"
#include "ui/pages/GameGrid.h"
#include "ui/widgets/RecentRow.h"

void MainWindow::onSearchChanged(const QString& query)
{
    m_grid->setSearch(query);
    m_recent->setSearch(query);
}
