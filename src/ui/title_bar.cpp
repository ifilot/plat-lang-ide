#include "title_bar.h"

#include <QColor>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenuBar>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QSvgRenderer>
#include <QWindow>

TitleBar::TitleBar(QMenuBar *menu_bar, QWidget *parent)
    : QWidget(parent),
      logo_label_(new QLabel(this)),
      title_label_(new QLabel(this)),
      minimize_button_(new QPushButton("-", this)),
      maximize_button_(new QPushButton("□", this)),
      close_button_(new QPushButton("×", this))
{
    setObjectName("customTitleBar");
    setFixedHeight(34);

    logo_label_->setObjectName("titleBarLogo");
    logo_label_->setFixedSize(24, 24);
    logo_label_->setAlignment(Qt::AlignCenter);

    title_label_->setObjectName("titleBarTitle");
    title_label_->setText("plat-lang IDE");
    title_label_->setFixedHeight(28);
    title_label_->setAlignment(Qt::AlignVCenter);

    menu_bar->setObjectName("titleBarMenu");
    menu_bar->setFixedHeight(28);
    menu_bar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    const QList<QPushButton *> controls = {
        minimize_button_, maximize_button_, close_button_
    };

    for (QPushButton *button : controls) {
        button->setFixedSize(42, 28);
        button->setFlat(true);
        button->setFocusPolicy(Qt::NoFocus);
    }

    close_button_->setObjectName("titleBarCloseButton");

    connect(minimize_button_, &QPushButton::clicked, this, [this]() {
        window()->showMinimized();
    });
    connect(maximize_button_, &QPushButton::clicked,
            this, &TitleBar::toggle_maximized);
    connect(close_button_, &QPushButton::clicked, this, [this]() {
        window()->close();
    });

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 3, 0, 3);
    layout->setSpacing(6);
    layout->addWidget(logo_label_, 0, Qt::AlignVCenter);
    layout->addWidget(title_label_, 0, Qt::AlignVCenter);
    layout->addWidget(menu_bar, 0, Qt::AlignVCenter);
    layout->addStretch(1);
    layout->addWidget(minimize_button_, 0, Qt::AlignVCenter);
    layout->addWidget(maximize_button_, 0, Qt::AlignVCenter);
    layout->addWidget(close_button_, 0, Qt::AlignVCenter);

    update_window_state();
    apply_theme(ThemeManager::Theme::Light);
}

void TitleBar::set_title(const QString &title)
{
    title_label_->setText(title);
}

void TitleBar::refresh_window_state()
{
    update_window_state();
}

void TitleBar::apply_theme(ThemeManager::Theme theme)
{
    set_logo_color(theme == ThemeManager::Theme::Dark
                       ? QColor("#ffffff")
                       : QColor("#000000"));
}

void TitleBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton || is_control_at(event->pos())) {
        QWidget::mousePressEvent(event);
        return;
    }

    QWindow *window_handle = window()->windowHandle();

    if (window_handle != nullptr) {
        window_handle->startSystemMove();
        event->accept();
        return;
    }

    QWidget::mousePressEvent(event);
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && !is_control_at(event->pos())) {
        toggle_maximized();
        event->accept();
        return;
    }

    QWidget::mouseDoubleClickEvent(event);
}

void TitleBar::toggle_maximized()
{
    if (window()->isMaximized()) {
        window()->showNormal();
    } else {
        window()->showMaximized();
    }

    update_window_state();
}

void TitleBar::update_window_state()
{
    maximize_button_->setText(window() != nullptr && window()->isMaximized()
                                  ? "❐"
                                  : "□");
}

bool TitleBar::is_control_at(const QPoint &position) const
{
    return minimize_button_->geometry().contains(position)
           || maximize_button_->geometry().contains(position)
           || close_button_->geometry().contains(position);
}

void TitleBar::set_logo_color(const QColor &color)
{
    constexpr int size = 22;
    QSvgRenderer renderer(QString(":/logos/plat-lang-logo.svg"));
    QPixmap alpha_mask(size, size);
    alpha_mask.fill(Qt::transparent);

    QPainter mask_painter(&alpha_mask);
    renderer.render(&mask_painter, QRectF(0, 0, size, size));
    mask_painter.end();

    QPixmap tinted(size, size);
    tinted.fill(Qt::transparent);

    QPainter painter(&tinted);
    painter.fillRect(tinted.rect(), color);
    painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    painter.drawPixmap(0, 0, alpha_mask);
    painter.end();

    logo_label_->setPixmap(tinted);
}
