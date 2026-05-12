#include "problems_panel.h"

#include <QEvent>
#include <QLabel>
#include <QVBoxLayout>

ProblemsPanel::ProblemsPanel(QWidget *parent)
    : QWidget(parent),
      empty_state_(new QLabel(this))
{
    retranslate_ui();
    empty_state_->setAlignment(Qt::AlignCenter);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->addWidget(empty_state_, 1);
}

void ProblemsPanel::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);

    if (event->type() == QEvent::LanguageChange) {
        retranslate_ui();
    }
}

void ProblemsPanel::retranslate_ui()
{
    empty_state_->setText(tr("No problems."));
}
