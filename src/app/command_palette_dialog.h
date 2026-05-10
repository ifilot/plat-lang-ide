#ifndef COMMAND_PALETTE_DIALOG_H
#define COMMAND_PALETTE_DIALOG_H

#include <QDialog>

class QListWidget;

/**
 * Lightweight command picker for common IDE actions.
 */
class CommandPaletteDialog : public QDialog {
    Q_OBJECT

public:
    /**
     * Creates the command palette.
     *
     * @param parent Optional parent widget.
     */
    explicit CommandPaletteDialog(QWidget *parent = nullptr);

    /**
     * Returns the selected command id.
     *
     * @return Command id, or empty string when none was selected.
     */
    QString selected_command() const;

private:
    /**
     * Adds one command row.
     *
     * @param title User-facing command title.
     * @param id Stable command id.
     */
    void add_command(const QString &title, const QString &id);

    QListWidget *commands_;
};

#endif
