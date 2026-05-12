#include "code_assistant_panel.h"

#include <QEvent>
#include <QLineEdit>
#include <QTextBrowser>
#include <QVBoxLayout>

CodeAssistantPanel::CodeAssistantPanel(QWidget *parent)
    : QWidget(parent),
      conversation_(new QTextBrowser(this)),
      prompt_input_(new QLineEdit(this))
{
    conversation_->setOpenExternalLinks(true);
    retranslate_ui();

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->addWidget(conversation_, 1);
    layout->addWidget(prompt_input_);
}

void CodeAssistantPanel::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);

    if (event->type() == QEvent::LanguageChange) {
        retranslate_ui();
    }
}

void CodeAssistantPanel::retranslate_ui()
{
    if (conversation_->document()->isEmpty()) {
        conversation_->setText(tr("Code assistant ready."));
    }

    prompt_input_->setPlaceholderText(tr("Ask about the current code..."));
}
