#ifndef CODE_EDITOR_H
#define CODE_EDITOR_H

#include <QPlainTextEdit>
#include <QTextEdit>
#include <QWidget>

#include <vector>

#include "theme_manager.h"

class CodeHighlighter;
class LineNumberArea;

/**
 * Represents one editable source document with line numbers and diagnostics.
 */
class CodeEditor : public QPlainTextEdit {
    Q_OBJECT

public:
    /**
     * Describes a source diagnostic shown in the editor.
     */
    struct Diagnostic {
        int line;
        QString message;
        QTextCharFormat format;
    };

    /**
     * Creates a code editor.
     *
     * @param parent Optional parent widget.
     */
    explicit CodeEditor(QWidget *parent = nullptr);

    /**
     * Replaces the diagnostics displayed in this editor.
     *
     * @param diagnostics Diagnostics keyed by one-based line number.
     */
    void set_diagnostics(const std::vector<Diagnostic> &diagnostics);

    /**
     * Applies syntax highlighting colors for the selected theme.
     *
     * @param theme Active application theme.
     */
    void apply_theme(ThemeManager::Theme theme);

    /**
     * Returns the width required by the line-number gutter.
     *
     * @return Gutter width in pixels.
     */
    int line_number_area_width() const;

    /**
     * Paints the line-number gutter.
     *
     * @param event Paint event for the gutter widget.
     */
    void paint_line_number_area(QPaintEvent *event);

protected:
    /**
     * Keeps the line-number area aligned with editor contents.
     *
     * @param event Resize event.
     */
    void resizeEvent(QResizeEvent *event) override;

private slots:
    /**
     * Updates the line-number gutter width when the document grows.
     *
     * @param block_count Current number of text blocks.
     */
    void update_line_number_area_width(int block_count);

    /**
     * Repaints the visible portion of the line-number gutter.
     *
     * @param rect Editor viewport rectangle that changed.
     * @param dy Vertical scroll delta.
     */
    void update_line_number_area(const QRect &rect, int dy);

    /**
     * Highlights the current cursor line.
     */
    void highlight_current_line();

private:
    /**
     * Rebuilds editor selections used for current-line and diagnostics display.
     */
    void refresh_extra_selections();

    ThemeManager::Theme theme_;
    LineNumberArea *line_number_area_;
    CodeHighlighter *highlighter_;
    std::vector<Diagnostic> diagnostics_;
};

/**
 * Gutter widget used by CodeEditor for line numbers.
 */
class LineNumberArea : public QWidget {
public:
    /**
     * Creates a line-number gutter for an editor.
     *
     * @param editor Editor that owns the gutter.
     */
    explicit LineNumberArea(CodeEditor *editor);

    /**
     * Returns the preferred gutter width.
     *
     * @return Width in pixels.
     */
    QSize sizeHint() const override;

protected:
    /**
     * Delegates gutter painting to the owning editor.
     *
     * @param event Paint event.
     */
    void paintEvent(QPaintEvent *event) override;

private:
    CodeEditor *editor_;
};

#endif
