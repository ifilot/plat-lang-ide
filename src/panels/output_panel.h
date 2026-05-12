#ifndef OUTPUT_PANEL_H
#define OUTPUT_PANEL_H

#include <QWidget>

class QTextBrowser;
class QEvent;

/**
 * Shows IDE-level informational output separate from program I/O.
 */
class OutputPanel : public QWidget {
    Q_OBJECT

public:
    /**
     * Creates the output panel.
     *
     * @param parent Optional parent widget.
     */
    explicit OutputPanel(QWidget *parent = nullptr);

    /**
     * Appends an informational line.
     *
     * @param text Text to append.
     */
    void append_message(const QString &text);

private:
    void changeEvent(QEvent *event) override;
    void retranslate_ui();

    QTextBrowser *output_;
};

#endif
