#include "bottom_panel.h"

#include <QEvent>
#include <QTabWidget>
#include <QVBoxLayout>

#include "output_panel.h"
#include "problems_panel.h"
#include "terminal_panel.h"

namespace {
constexpr int terminal_tab_index = 0;
constexpr int problems_tab_index = 1;
constexpr int output_tab_index = 2;
}

BottomPanel::BottomPanel(QWidget *parent)
    : QWidget(parent),
      tabs_(new QTabWidget(this)),
      terminal_panel_(new TerminalPanel(this)),
      problems_panel_(new ProblemsPanel(this)),
      output_panel_(new OutputPanel(this))
{
    tabs_->addTab(terminal_panel_, QString());
    tabs_->addTab(problems_panel_, QString());
    tabs_->addTab(output_panel_, QString());
    retranslate_ui();
    tabs_->setCurrentIndex(terminal_tab_index);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(tabs_, 1);
}

void BottomPanel::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);

    if (event->type() == QEvent::LanguageChange) {
        retranslate_ui();
    }
}

void BottomPanel::retranslate_ui()
{
    tabs_->setTabText(terminal_tab_index, tr("Terminal"));
    tabs_->setTabText(problems_tab_index, tr("Problems"));
    tabs_->setTabText(output_tab_index, tr("Output"));
}

TerminalPanel *BottomPanel::terminal_panel() const
{
    return terminal_panel_;
}

void BottomPanel::append_output_message(const QString &text)
{
    output_panel_->append_message(text);
}

void BottomPanel::set_tab_visible(Tab tab, bool visible)
{
    int index = tab_index(tab);
    tabs_->setTabVisible(index, visible);

    if (visible) {
        tabs_->setCurrentIndex(index);
    }

    update_visibility();
}

bool BottomPanel::is_tab_visible(Tab tab) const
{
    return tabs_->isTabVisible(tab_index(tab));
}

int BottomPanel::tab_index(Tab tab) const
{
    switch (tab) {
    case Tab::Terminal:
        return terminal_tab_index;
    case Tab::Problems:
        return problems_tab_index;
    case Tab::Output:
        return output_tab_index;
    }

    return terminal_tab_index;
}

void BottomPanel::update_visibility()
{
    bool any_visible = tabs_->isTabVisible(terminal_tab_index)
        || tabs_->isTabVisible(problems_tab_index)
        || tabs_->isTabVisible(output_tab_index);

    setVisible(any_visible);

    if (!any_visible || tabs_->isTabVisible(tabs_->currentIndex())) {
        return;
    }

    if (tabs_->isTabVisible(terminal_tab_index)) {
        tabs_->setCurrentIndex(terminal_tab_index);
    } else if (tabs_->isTabVisible(problems_tab_index)) {
        tabs_->setCurrentIndex(problems_tab_index);
    } else {
        tabs_->setCurrentIndex(output_tab_index);
    }
}
