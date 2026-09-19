#include "ui/pages/Store.h"
#include "ui/Theme.h"
#include "ui/Style.h"
#include "ui/Fonts.h"

using namespace theme;
using namespace theme::C;

static const QStringList STORES = {"Steam", "Epic Games", "GOG", "itch.io"};
static const QUrl STEAM_URL("https://store.steampowered.com/");

static QString buildRecolorJs() {
    return QStringLiteral(
        "(function(){"
        "var old=document.getElementById('spacetheme-css');"
        "if(old)old.remove();"
        "var s=document.createElement('style');"
        "s.id='spacetheme-css';"
        "s.textContent='body{background:#111111!important;color:#fff!important;}'"
        ";document.head.appendChild(s);"
        "})()"
    );
}

static const QString STEAM_RECOLOR_JS = buildRecolorJs();

static const QString MUTATION_OBSERVER_JS =
    QStringLiteral(
        "(function(){"
        "if(window.__spacetheme_observer)return;"
        "window.__spacetheme_observer=true;"
        "var reinject=function(){"
        "if(!document.getElementById('spacetheme-css')){")
    + STEAM_RECOLOR_JS +
    QStringLiteral(
        "}"
        "};"
        "var obs=new MutationObserver(reinject);"
        "obs.observe(document.documentElement,{childList:true,subtree:true});"
        "})()"
    );

// -- WebStorePane ----------------------------------------------------------

WebStorePane::WebStorePane(const QUrl& url, QWidget* parent)
    : QWidget(parent), m_url(url)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_view = new QWebEngineView();
    m_view->setStyleSheet("background-color: #111111;");
    m_view->setVisible(false);
    connect(m_view, &QWebEngineView::loadFinished, this, &WebStorePane::onLoadFinished);
    layout->addWidget(m_view);
}

void WebStorePane::load() {
    if (!m_view) return;
    m_cssInjected = false;
    m_cssRetries = 0;
    m_view->setVisible(false);
    m_view->load(m_url);
}

void WebStorePane::onLoadFinished(bool ok) {
    if (!m_view || !ok) return;
    m_view->page()->runJavaScript(STEAM_RECOLOR_JS);
    m_view->page()->runJavaScript(MUTATION_OBSERVER_JS);
    m_view->page()->runJavaScript(
        "document.getElementById('spacetheme-css') !== null",
        [this](const QVariant& result) {
            bool found = result.toBool();
            if (found && !m_cssInjected) {
                m_cssInjected = true;
                m_view->setVisible(true);
            } else if (!found) {
                QTimer::singleShot(200, this, &WebStorePane::retryCss);
            }
        }
    );
}

void WebStorePane::retryCss() {
    if (!m_view || m_cssInjected) return;
    ++m_cssRetries;
    if (m_cssRetries > CSS_MAX_RETRIES) {
        m_view->setVisible(true);
        return;
    }
    m_view->page()->runJavaScript(STEAM_RECOLOR_JS);
    m_view->page()->runJavaScript(MUTATION_OBSERVER_JS);
    m_view->page()->runJavaScript(
        "document.getElementById('spacetheme-css') !== null",
        [this](const QVariant& result) {
            bool found = result.toBool();
            if (found && !m_cssInjected) {
                m_cssInjected = true;
                m_view->setVisible(true);
            } else if (!found) {
                QTimer::singleShot(200, this, &WebStorePane::retryCss);
            }
        }
    );
}

// -- StorePage -------------------------------------------------------------

StorePage::StorePage(QWidget* parent)
    : QWidget(parent)
{
    setStyleSheet(QStringLiteral("background-color: %1;").arg(C::BG));
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 18, 24, 18);
    layout->setSpacing(12);

    auto* header = new QLabel("Store");
    QFont headerFont(fonts::UI_FONT, 22, QFont::Bold);
    header->setFont(headerFont);
    style::label(header, C::TEXT);
    layout->addWidget(header);

    m_stack = new QStackedWidget();
    m_webPane = new WebStorePane(STEAM_URL);
    m_stack->addWidget(m_webPane);
    for (int i = 1; i < STORES.size(); ++i)
        m_stack->addWidget(buildPlaceholder(STORES[i]));
    layout->addWidget(buildTabs());
    layout->addWidget(m_stack, 1);

    m_webPane->load();
}

QWidget* StorePage::buildTabs() {
    auto* bar = new QWidget();
    bar->setStyleSheet(QStringLiteral("background-color: %1; border-radius: %2px;").arg(C::COLOR_2).arg(C::RADIUS));
    auto* barLayout = new QHBoxLayout(bar);
    barLayout->setContentsMargins(6, 6, 6, 6);
    barLayout->setSpacing(6);

    m_tabsGroup = new QButtonGroup(this);
    m_tabsGroup->setExclusive(true);
    for (int i = 0; i < STORES.size(); ++i) {
        auto* btn = new QPushButton(STORES[i]);
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(QStringLiteral(
            "QPushButton { background-color: %1; color: #a9a9a9; border: none; "
            "border-radius: %2px; padding: 6px 16px; font-size: 14px; } "
            "QPushButton:hover { background-color: %3; color: %4; } "
            "QPushButton:checked { background-color: %5; color: %6; font-weight: 600; }")
            .arg(C::COLOR_3).arg(C::RADIUS).arg(C::COLOR_4).arg(C::TEXT).arg(C::ACCENT_1).arg(C::TEXT));
        m_tabsGroup->addButton(btn, i);
        barLayout->addWidget(btn);
    }
    connect(m_tabsGroup, &QButtonGroup::buttonClicked, this, &StorePage::switchTab);
    m_tabsGroup->button(0)->setChecked(true);
    return bar;
}

void StorePage::switchTab(QAbstractButton* button) {
    int index = m_tabsGroup->id(button);
    if (index < 0) return;
    m_stack->setCurrentIndex(index);
    if (index == 0) m_webPane->load();
}

QWidget* StorePage::buildPlaceholder(const QString& name) {
    auto* page = new QFrame();
    style::panel(page, 2);
    auto* v = new QVBoxLayout(page);
    v->addStretch(1);
    auto* store = new QLabel(name);
    store->setAlignment(Qt::AlignCenter);
    style::label(store, C::TEXT_DIM, 22, 700);
    auto* note = new QLabel("Store integration coming soon");
    note->setAlignment(Qt::AlignCenter);
    style::label(note, C::TEXT_DIM, 13);
    v->addWidget(store);
    v->addWidget(note);
    v->addStretch(1);
    return page;
}
