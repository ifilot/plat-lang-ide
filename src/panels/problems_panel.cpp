#include "problems_panel.h"

#include <QLabel>
#include <QVBoxLayout>

ProblemsPanel::ProblemsPanel(QWidget *parent)
    : QWidget(parent),
      empty_state_(new QLabel("No problems.", this))
{
    empty_state_->setAlignment(Qt::AlignCenter);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->addWidget(empty_state_, 1);
}
