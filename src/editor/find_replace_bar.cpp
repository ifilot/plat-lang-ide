#include "find_replace_bar.h"

#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSize>
#include <QToolButton>

FindReplaceBar::FindReplaceBar(QWidget *parent)
    : QWidget(parent),
      find_input_(new QLineEdit(this)),
      replace_label_(new QLabel("Replace", this)),
      replace_input_(new QLineEdit(this)),
      replace_button_(new QPushButton("Replace", this)),
      replace_all_button_(new QPushButton("Replace All", this))
{
    setObjectName("findReplaceBar");
    setVisible(false);

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 5, 8, 5);
    layout->setSpacing(6);

    auto *find_label = new QLabel("Find", this);
    find_input_->setObjectName("findInput");
    find_input_->setClearButtonEnabled(true);

    auto *previous_button = new QPushButton("Previous", this);
    previous_button->setObjectName("findBarButton");

    auto *next_button = new QPushButton("Next", this);
    next_button->setObjectName("findBarButton");

    replace_input_->setObjectName("replaceInput");
    replace_input_->setClearButtonEnabled(true);

    replace_button_->setObjectName("findBarButton");
    replace_all_button_->setObjectName("findBarButton");

    auto *close_button = new QToolButton(this);
    close_button->setObjectName("findBarCloseButton");
    close_button->setIcon(QIcon(":/assets/icons/fontawesome/solid/xmark.svg"));
    close_button->setIconSize(QSize(9, 9));
    close_button->setFixedSize(22, 22);
    close_button->setAutoRaise(true);
    close_button->setToolTip("Close");

    layout->addWidget(find_label);
    layout->addWidget(find_input_, 1);
    layout->addWidget(previous_button);
    layout->addWidget(next_button);
    layout->addSpacing(8);
    layout->addWidget(replace_label_);
    layout->addWidget(replace_input_, 1);
    layout->addWidget(replace_button_);
    layout->addWidget(replace_all_button_);
    layout->addWidget(close_button);

    connect(find_input_, &QLineEdit::returnPressed,
            this, &FindReplaceBar::find_next_requested);
    connect(replace_input_, &QLineEdit::returnPressed,
            this, &FindReplaceBar::replace_requested);
    connect(previous_button, &QPushButton::clicked,
            this, &FindReplaceBar::find_previous_requested);
    connect(next_button, &QPushButton::clicked,
            this, &FindReplaceBar::find_next_requested);
    connect(replace_button_, &QPushButton::clicked,
            this, &FindReplaceBar::replace_requested);
    connect(replace_all_button_, &QPushButton::clicked,
            this, &FindReplaceBar::replace_all_requested);
    connect(close_button, &QToolButton::clicked, this, &QWidget::hide);
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
