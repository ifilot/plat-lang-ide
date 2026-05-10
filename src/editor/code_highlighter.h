#ifndef CODE_HIGHLIGHTER_H
#define CODE_HIGHLIGHTER_H

#include <QRegularExpression>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>

#include <vector>

#include "theme_manager.h"

class QTextDocument;

/**
 * Applies lightweight syntax highlighting for plat-lang source code.
 */
class CodeHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    /**
     * Creates a syntax highlighter for a text document.
     *
     * @param parent Document to highlight.
     */
    explicit CodeHighlighter(QTextDocument *parent);

    /**
     * Applies syntax colors for the selected application theme.
     *
     * @param theme Active application theme.
     */
    void apply_theme(ThemeManager::Theme theme);

protected:
    /**
     * Highlights one block of source text.
     *
     * @param text Text block to highlight.
     */
    void highlightBlock(const QString &text) override;

private:
    struct HighlightRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    /**
     * Rebuilds regex highlight rules from current text formats.
     */
    void rebuild_rules();

    std::vector<HighlightRule> rules_;
    QTextCharFormat builtin_format_;
    QTextCharFormat comment_format_;
    QTextCharFormat keyword_format_;
    QTextCharFormat literal_format_;
    QTextCharFormat logical_format_;
    QTextCharFormat number_format_;
    QTextCharFormat operator_format_;
    QTextCharFormat string_format_;
};

#endif
