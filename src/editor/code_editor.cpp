#include "code_editor.h"

#include <QPalette>
#include <QPainter>
#include <QPaintEvent>
#include <QTextBlock>

#include "app_fonts.h"
#include "code_highlighter.h"

LineNumberArea::LineNumberArea(CodeEditor *editor)
    : QWidget(editor),
      editor_(editor)
{
}

QSize LineNumberArea::sizeHint() const
{
    return QSize(editor_->line_number_area_width(), 0);
}

void LineNumberArea::paintEvent(QPaintEvent *event)
{
    editor_->paint_line_number_area(event);
}

CodeEditor::CodeEditor(QWidget *parent)
    : QPlainTextEdit(parent),
      line_number_area_(new LineNumberArea(this)),
      highlighter_(new CodeHighlighter(document()))
{
    setFont(AppFonts::code_font(11));
    setLineWrapMode(QPlainTextEdit::NoWrap);
    setTabStopDistance(fontMetrics().horizontalAdvance(' ') * 4);

    connect(this, &CodeEditor::blockCountChanged,
            this, &CodeEditor::update_line_number_area_width);
    connect(this, &CodeEditor::updateRequest,
            this, &CodeEditor::update_line_number_area);
    connect(this, &CodeEditor::cursorPositionChanged,
            this, &CodeEditor::highlight_current_line);

    update_line_number_area_width(0);
    highlight_current_line();
}

void CodeEditor::set_diagnostics(const std::vector<Diagnostic> &diagnostics)
{
    diagnostics_ = diagnostics;
    refresh_extra_selections();
}

void CodeEditor::apply_theme(ThemeManager::Theme theme)
{
    highlighter_->apply_theme(theme);
    refresh_extra_selections();
    line_number_area_->update();
}

int CodeEditor::line_number_area_width() const
{
    int digits = 1;
    int max = qMax(1, blockCount());

    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    return 12 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
}

void CodeEditor::paint_line_number_area(QPaintEvent *event)
{
    QPainter painter(line_number_area_);
    painter.fillRect(event->rect(), palette().alternateBase());

    QTextBlock block = firstVisibleBlock();
    int block_number = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(block_number + 1);
            painter.setPen(palette().placeholderText().color());
            painter.drawText(0, top, line_number_area_->width() - 5,
                             fontMetrics().height(), Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++block_number;
    }
}

void CodeEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);

    QRect content_rect = contentsRect();
    line_number_area_->setGeometry(QRect(content_rect.left(), content_rect.top(),
                                         line_number_area_width(),
                                         content_rect.height()));
}

void CodeEditor::update_line_number_area_width(int block_count)
{
    Q_UNUSED(block_count);
    setViewportMargins(line_number_area_width(), 0, 0, 0);
}

void CodeEditor::update_line_number_area(const QRect &rect, int dy)
{
    if (dy != 0) {
        line_number_area_->scroll(0, dy);
    } else {
        line_number_area_->update(0, rect.y(), line_number_area_->width(),
                                  rect.height());
    }

    if (rect.contains(viewport()->rect())) {
        update_line_number_area_width(0);
    }
}

void CodeEditor::highlight_current_line()
{
    refresh_extra_selections();
}

void CodeEditor::refresh_extra_selections()
{
    QList<QTextEdit::ExtraSelection> selections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection current_line;
        QColor current_line_color = palette().highlight().color();
        current_line_color.setAlpha(36);
        current_line.format.setBackground(current_line_color);
        current_line.format.setProperty(QTextFormat::FullWidthSelection, true);
        current_line.cursor = textCursor();
        current_line.cursor.clearSelection();
        selections.append(current_line);
    }

    for (const Diagnostic &diagnostic : diagnostics_) {
        QTextBlock block = document()->findBlockByNumber(diagnostic.line - 1);

        if (!block.isValid()) {
            continue;
        }

        QTextEdit::ExtraSelection selection;
        selection.cursor = QTextCursor(block);
        selection.cursor.select(QTextCursor::LineUnderCursor);
        selection.format = diagnostic.format;
        selection.format.setToolTip(diagnostic.message);
        selections.append(selection);
    }

    setExtraSelections(selections);
}
