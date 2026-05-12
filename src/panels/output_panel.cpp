#include "output_panel.h"

#include <QEvent>
#include <QTextBrowser>
#include <QVBoxLayout>

OutputPanel::OutputPanel(QWidget *parent)
    : QWidget(parent),
      output_(new QTextBrowser(this))
{
    output_->setOpenExternalLinks(true);
    retranslate_ui();

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->addWidget(output_, 1);
}

void OutputPanel::append_message(const QString &text)
{
    output_->append(text.toHtmlEscaped());
}

void OutputPanel::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);

    if (event->type() == QEvent::LanguageChange) {
        retranslate_ui();
    }
}

void OutputPanel::retranslate_ui()
{
    if (output_->document()->isEmpty()) {
        output_->setText(tr("IDE output ready."));
    }
}
