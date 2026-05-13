#include "code_highlighter.h"

#include <QColor>
#include <QFont>

namespace {
enum class SyntaxRole {
    Builtin,
    Keyword,
    Literal,
    Logical,
    Type,
};

struct SyntaxWord {
    const char *word;
    SyntaxRole role;
};

// Hand-edit this table to add, remove, rename, or recolor platlang words.
// The role controls which syntax color/style is applied in apply_theme().
constexpr SyntaxWord kSyntaxWords[] = {
    {"aafbraeke", SyntaxRole::Keyword},
    {"aafdrokke", SyntaxRole::Builtin},
    {"aafdrökke", SyntaxRole::Builtin},
    {"aafdrökke", SyntaxRole::Builtin},
    {"angesj", SyntaxRole::Keyword},
    {"canvas", SyntaxRole::Builtin},
    {"canvas_lien", SyntaxRole::Builtin},
    {"canvas_ope", SyntaxRole::Builtin},
    {"canvas_pauze", SyntaxRole::Builtin},
    {"canvas_rechhook", SyntaxRole::Builtin},
    {"canvas_sirkel", SyntaxRole::Builtin},
    {"canvas_sloet", SyntaxRole::Builtin},
    {"canvas_teks", SyntaxRole::Builtin},
    {"canvas_toon", SyntaxRole::Builtin},
    {"canvas_wach", SyntaxRole::Builtin},
    {"canvas_wis", SyntaxRole::Builtin},
    {"en", SyntaxRole::Logical},
    {"enj", SyntaxRole::Keyword},
    {"es", SyntaxRole::Keyword},
    {"euversjlaon", SyntaxRole::Keyword},
    {"funksie", SyntaxRole::Keyword},
    {"inlaaje", SyntaxRole::Keyword},
    {"inveure", SyntaxRole::Builtin},
    {"loat", SyntaxRole::Keyword},
    {"kwatsj", SyntaxRole::Literal},
    {"niks", SyntaxRole::Literal},
    {"nommer", SyntaxRole::Type},
    {"nómmer", SyntaxRole::Type},
    {"nómmer", SyntaxRole::Type},
    {"of", SyntaxRole::Logical},
    {"tabel", SyntaxRole::Type},
    {"teks", SyntaxRole::Type},
    {"trok", SyntaxRole::Keyword},
    {"trök", SyntaxRole::Keyword},
    {"veur", SyntaxRole::Keyword},
    {"waatis", SyntaxRole::Builtin},
    {"woar", SyntaxRole::Literal},
    {"zolang", SyntaxRole::Keyword},
};

QString whole_word_pattern(const char *word)
{
    const QString identifier_character =
        QStringLiteral("[\\p{L}\\p{M}\\p{N}_]");
    const QString escaped_word = QRegularExpression::escape(
        QString::fromUtf8(word));

    return QStringLiteral("(?<!%1)%2(?!%1)")
        .arg(identifier_character, escaped_word);
}

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
    const ThemeManager::SyntaxColors &syntax =
        ThemeManager::theme_definition(theme).syntax;

    keyword_format_.setForeground(QColor(syntax.keyword));
    literal_format_.setForeground(QColor(syntax.constant));
    logical_format_.setForeground(QColor(syntax.operator_color));
    builtin_format_.setForeground(QColor(syntax.function));
    type_format_.setForeground(QColor(syntax.type));
    string_format_.setForeground(QColor(syntax.string));
    number_format_.setForeground(QColor(syntax.number));
    operator_format_.setForeground(QColor(syntax.operator_color));
    comment_format_.setForeground(QColor(syntax.comment));

    keyword_format_.setFontWeight(QFont::Bold);
    literal_format_.setFontWeight(QFont::Bold);
    logical_format_.setFontItalic(true);
    builtin_format_.setFontWeight(QFont::Bold);
    type_format_.setFontWeight(QFont::Bold);
    operator_format_.setFontWeight(QFont::Bold);
    comment_format_.setFontItalic(true);

    rebuild_rules();
    rehighlight();
}

void CodeHighlighter::rebuild_rules()
{
    rules_.clear();

    for (const SyntaxWord &syntax_word : kSyntaxWords) {
        QTextCharFormat format;

        switch (syntax_word.role) {
        case SyntaxRole::Builtin:
            format = builtin_format_;
            break;
        case SyntaxRole::Keyword:
            format = keyword_format_;
            break;
        case SyntaxRole::Literal:
            format = literal_format_;
            break;
        case SyntaxRole::Logical:
            format = logical_format_;
            break;
        case SyntaxRole::Type:
            format = type_format_;
            break;
        }

        rules_.push_back({
            QRegularExpression(whole_word_pattern(syntax_word.word)),
            format
        });
    }
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
