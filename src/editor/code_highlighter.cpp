#include "code_highlighter.h"

#include <QColor>
#include <QFont>
#include <QStringList>

namespace {
/**
 * Finds the first platlang comment marker outside a string literal.
 *
 * @param text Source line to inspect.
 * @return Comment start index, or -1 when the line has no comment.
 */
int find_comment_start(const QString &text)
{
    bool in_string = false;
    bool escaped = false;

    for (int index = 0; index < text.length(); ++index) {
        QChar character = text[index];

        if (in_string) {
            if (escaped) {
                escaped = false;
                continue;
            }

            if (character == '\\') {
                escaped = true;
                continue;
            }

            if (character == '"') {
                in_string = false;
            }

            continue;
        }

        if (character == '"') {
            in_string = true;
            continue;
        }

        if (character == '#') {
            return index;
        }
    }

    return -1;
}
}

CodeHighlighter::CodeHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    apply_theme(ThemeManager::load_theme());
}

void CodeHighlighter::apply_theme(ThemeManager::Theme theme)
{
    if (theme == ThemeManager::Theme::Dark) {
        keyword_format_.setForeground(QColor(122, 162, 247));
        literal_format_.setForeground(QColor(187, 154, 247));
        logical_format_.setForeground(QColor(122, 162, 247));
        builtin_format_.setForeground(QColor(125, 207, 255));
        string_format_.setForeground(QColor(158, 206, 106));
        number_format_.setForeground(QColor(255, 158, 100));
        operator_format_.setForeground(QColor(192, 202, 245));
        comment_format_.setForeground(QColor(86, 95, 137));
    } else {
        keyword_format_.setForeground(QColor(32, 82, 149));
        literal_format_.setForeground(QColor(108, 74, 160));
        logical_format_.setForeground(QColor(32, 82, 149));
        builtin_format_.setForeground(QColor(118, 72, 158));
        string_format_.setForeground(QColor(146, 72, 34));
        number_format_.setForeground(QColor(84, 120, 42));
        operator_format_.setForeground(QColor(80, 86, 96));
        comment_format_.setForeground(QColor(100, 115, 125));
    }

    keyword_format_.setFontWeight(QFont::Bold);
    literal_format_.setFontWeight(QFont::Bold);
    logical_format_.setFontItalic(true);
    builtin_format_.setFontWeight(QFont::Bold);
    operator_format_.setFontWeight(QFont::Bold);
    comment_format_.setFontItalic(true);

    rebuild_rules();
    rehighlight();
}

void CodeHighlighter::rebuild_rules()
{
    rules_.clear();

    const QStringList keywords = {
        "aafbraeke", "angesj", "enj", "es", "funksie", "loat", "trok",
        "veur", "weier", "zolang"
    };

    for (const QString &keyword : keywords) {
        rules_.push_back({QRegularExpression("\\b" + keyword + "\\b"),
                          keyword_format_});
    }

    const QStringList literals = {"neetwoar", "niks", "woar"};

    for (const QString &literal : literals) {
        rules_.push_back({QRegularExpression("\\b" + literal + "\\b"),
                          literal_format_});
    }

    rules_.push_back({QRegularExpression("\\b(and|or)\\b"), logical_format_});
    rules_.push_back({QRegularExpression("\\baafdrokke\\b"), builtin_format_});
}

void CodeHighlighter::highlightBlock(const QString &text)
{
    int comment_start = find_comment_start(text);
    int code_length = comment_start >= 0 ? comment_start : text.length();

    for (const HighlightRule &rule : rules_) {
        QRegularExpressionMatchIterator matches =
            rule.pattern.globalMatch(text.left(code_length));

        while (matches.hasNext()) {
            QRegularExpressionMatch match = matches.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    QRegularExpression number_pattern("\\b[0-9]+(?:\\.[0-9]+)?\\b");
    QRegularExpressionMatchIterator number_matches =
        number_pattern.globalMatch(text.left(code_length));

    while (number_matches.hasNext()) {
        QRegularExpressionMatch match = number_matches.next();
        setFormat(match.capturedStart(), match.capturedLength(), number_format_);
    }

    QRegularExpression operator_pattern(
        "==|!=|<=|>=|[+\\-*/%!<>=:,.(){}\\[\\]]");
    QRegularExpressionMatchIterator operator_matches =
        operator_pattern.globalMatch(text.left(code_length));

    while (operator_matches.hasNext()) {
        QRegularExpressionMatch match = operator_matches.next();
        setFormat(match.capturedStart(), match.capturedLength(), operator_format_);
    }

    QRegularExpression string_pattern("\"([^\\\\\"\\n]|\\\\[nt\"\\\\])*\"");
    QRegularExpressionMatchIterator string_matches =
        string_pattern.globalMatch(text.left(code_length));

    while (string_matches.hasNext()) {
        QRegularExpressionMatch match = string_matches.next();
        setFormat(match.capturedStart(), match.capturedLength(), string_format_);
    }

    if (comment_start >= 0) {
        setFormat(comment_start, text.length() - comment_start, comment_format_);
    }
}
