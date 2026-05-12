#include "find_replace_bar.h"

#include <QHBoxLayout>
#include <QIcon>
#include <QEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSize>
#include <QToolButton>

FindReplaceBar::FindReplaceBar(QWidget *parent)
    : QWidget(parent),
      find_label_(new QLabel(this)),
      find_input_(new QLineEdit(this)),
      replace_label_(new QLabel(this)),
      replace_input_(new QLineEdit(this)),
      previous_button_(new QPushButton(this)),
      next_button_(new QPushButton(this)),
      replace_button_(new QPushButton(this)),
      replace_all_button_(new QPushButton(this)),
      close_button_(new QToolButton(this))
{
    setObjectName("findReplaceBar");
    setVisible(false);
    retranslate_ui();

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 5, 8, 5);
    layout->setSpacing(6);

    find_input_->setObjectName("findInput");
    find_input_->setClearButtonEnabled(true);

    previous_button_->setObjectName("findBarButton");

    next_button_->setObjectName("findBarButton");

    replace_input_->setObjectName("replaceInput");
    replace_input_->setClearButtonEnabled(true);

    replace_button_->setObjectName("findBarButton");
    replace_all_button_->setObjectName("findBarButton");

    close_button_->setObjectName("findBarCloseButton");
    close_button_->setIcon(QIcon(":/assets/icons/fontawesome/solid/xmark.svg"));
    close_button_->setIconSize(QSize(9, 9));
    close_button_->setFixedSize(22, 22);
    close_button_->setAutoRaise(true);

    layout->addWidget(find_label_);
    layout->addWidget(find_input_, 1);
    layout->addWidget(previous_button_);
    layout->addWidget(next_button_);
    layout->addSpacing(8);
    layout->addWidget(replace_label_);
    layout->addWidget(replace_input_, 1);
    layout->addWidget(replace_button_);
    layout->addWidget(replace_all_button_);
    layout->addWidget(close_button_);

    connect(find_input_, &QLineEdit::returnPressed,
            this, &FindReplaceBar::find_next_requested);
    connect(replace_input_, &QLineEdit::returnPressed,
            this, &FindReplaceBar::replace_requested);
    connect(previous_button_, &QPushButton::clicked,
            this, &FindReplaceBar::find_previous_requested);
    connect(next_button_, &QPushButton::clicked,
            this, &FindReplaceBar::find_next_requested);
    connect(replace_button_, &QPushButton::clicked,
            this, &FindReplaceBar::replace_requested);
    connect(replace_all_button_, &QPushButton::clicked,
            this, &FindReplaceBar::replace_all_requested);
    connect(close_button_, &QToolButton::clicked, this, &QWidget::hide);
}

void FindReplaceBar::show_find()
{
    replace_label_->setVisible(false);
    replace_input_->setVisible(false);
    replace_button_->setVisible(false);
    replace_all_button_->setVisible(false);

    setVisible(true);
    find_input_->setFocus();
    find_input_->selectAll();
}

void FindReplaceBar::show_replace()
{
    replace_label_->setVisible(true);
    replace_input_->setVisible(true);
    replace_button_->setVisible(true);
    replace_all_button_->setVisible(true);

    setVisible(true);
    find_input_->setFocus();
    find_input_->selectAll();
}

QString FindReplaceBar::find_text() const
{
    return find_input_->text();
}

QString FindReplaceBar::replace_text() const
{
    return replace_input_->text();
}

void FindReplaceBar::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);

    if (event->type() == QEvent::LanguageChange) {
        retranslate_ui();
    }
}

void FindReplaceBar::retranslate_ui()
{
    find_label_->setText(tr("Find"));
    previous_button_->setText(tr("Previous"));
    next_button_->setText(tr("Next"));
    replace_label_->setText(tr("Replace"));
    replace_button_->setText(tr("Replace"));
    replace_all_button_->setText(tr("Replace All"));
    close_button_->setToolTip(tr("Close"));
}
