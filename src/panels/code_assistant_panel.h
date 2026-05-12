#ifndef CODE_ASSISTANT_PANEL_H
#define CODE_ASSISTANT_PANEL_H

#include <QWidget>

class QTextBrowser;
class QLineEdit;
class QEvent;

/**
 * Provides the code assistant conversation panel.
 */
class CodeAssistantPanel : public QWidget {
    Q_OBJECT

public:
    /**
     * Creates the code assistant panel.
     *
     * @param parent Optional parent widget.
     */
    explicit CodeAssistantPanel(QWidget *parent = nullptr);

private:
    void changeEvent(QEvent *event) override;
    void retranslate_ui();

    QTextBrowser *conversation_;
    QLineEdit *prompt_input_;
};

#endif
