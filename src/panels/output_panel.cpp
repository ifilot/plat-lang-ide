#include "output_panel.h"

#include <QTextBrowser>
#include <QVBoxLayout>

OutputPanel::OutputPanel(QWidget *parent)
    : QWidget(parent),
      output_(new QTextBrowser(this))
{
    output_->setOpenExternalLinks(true);
    output_->setText("IDE output ready.");

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->addWidget(output_, 1);
}

void OutputPanel::append_message(const QString &text)
{
    output_->append(text.toHtmlEscaped());
}
