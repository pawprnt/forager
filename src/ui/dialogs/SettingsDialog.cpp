#include "ui/dialogs/SettingsDialog.h"
#include "ui/Theme.h"
#include "ui/Style.h"
#include "core/Config.h"
#include <QLineEdit>

using namespace theme;
using namespace theme::C;

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Settings");
    resize(760, 560);
    setMinimumSize(640, 500);
    style::background(this, COLOR_2);

    auto* v = style::vbox(this);

    v->addWidget(buildHeader());

    auto* body = new QHBoxLayout();
    body->setContentsMargins(16, 16, 16, 12);
    body->setSpacing(16);
    body->addWidget(buildNav());

    m_pages = new QStackedWidget();
    style::transparent(m_pages);

    m_libraryTab = buildLibraryTab();
    m_pages->addWidget(m_libraryTab);

    m_protonTab = buildProtonTab();
    m_pages->addWidget(m_protonTab);

    m_accountTab = buildAccountTab();
    m_pages->addWidget(m_accountTab);

    auto* content = new QFrame();
    content->setObjectName("SettingsContent");
    style::panel(content, 2);
    auto* contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(20, 20, 20, 20);
    contentLayout->addWidget(m_pages);
    body->addWidget(content, 1);

    v->addLayout(body);
    v->addWidget(buildFooter());
}

QString SettingsDialog::selectedCardSize() const {
    return Config::instance().displaySize();
}

QString SettingsDialog::gamesDirText() const {
    return m_dirEdit ? m_dirEdit->text() : Config::instance().gamesDir();
}

QString SettingsDialog::steamAppcacheText() const {
    return m_cacheEdit ? m_cacheEdit->text() : Config::instance().steamAppcache();
}

void SettingsDialog::save() {
    if (m_dirEdit)
        Config::instance().setGamesDir(m_dirEdit->text());
    if (m_cacheEdit)
        Config::instance().setSteamAppcache(m_cacheEdit->text());
    accept();
}
