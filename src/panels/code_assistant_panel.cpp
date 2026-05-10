#include "code_assistant_panel.h"

#include <QLineEdit>
#include <QTextBrowser>
#include <QVBoxLayout>

CodeAssistantPanel::CodeAssistantPanel(QWidget *parent)
    : QWidget(parent),
      conversation_(new QTextBrowser(this)),
      prompt_input_(new QLineEdit(this))
{
    conversation_->setOpenExternalLinks(true);
    conversation_->setText("Code assistant ready.");
    prompt_input_->setPlaceholderText("Ask about the current code...");

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->addWidget(conversation_, 1);
    layout->addWidget(prompt_input_);
}
