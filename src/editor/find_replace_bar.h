#ifndef FIND_REPLACE_BAR_H
#define FIND_REPLACE_BAR_H

#include <QWidget>

class QLineEdit;
class QLabel;
class QPushButton;
class QToolButton;
class QEvent;

/**
 * Compact find and replace controls for the active editor.
 */
class FindReplaceBar : public QWidget {
    Q_OBJECT

public:
    /**
     * Creates the find and replace bar.
     *
     * @param parent Optional parent widget.
     */
    explicit FindReplaceBar(QWidget *parent = nullptr);

    /**
     * Shows the bar in find mode.
     */
    void show_find();

    /**
     * Shows the bar in replace mode.
     */
    void show_replace();

    /**
     * Returns the current find text.
     *
     * @return Find input contents.
     */
    QString find_text() const;

    /**
     * Returns the current replacement text.
     *
     * @return Replacement input contents.
     */
    QString replace_text() const;

signals:
    /**
     * Requests the next match.
     */
    void find_next_requested();

    /**
     * Requests the previous match.
     */
    void find_previous_requested();

    /**
     * Requests replacement of the current match.
     */
    void replace_requested();

    /**
     * Requests replacement of all matches.
     */
    void replace_all_requested();

private:
    void changeEvent(QEvent *event) override;
    void retranslate_ui();

    QLabel *find_label_;
    QLineEdit *find_input_;
    QLabel *replace_label_;
    QLineEdit *replace_input_;
    QPushButton *previous_button_;
    QPushButton *next_button_;
    QPushButton *replace_button_;
    QPushButton *replace_all_button_;
    QToolButton *close_button_;
};

#endif
