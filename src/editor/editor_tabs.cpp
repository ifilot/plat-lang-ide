#include "editor_tabs.h"

#include <QColor>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QStringList>
#include <QTabBar>
#include <QTextCursor>
#include <QTextDocument>
#include <QToolButton>

#include <algorithm>

#include "code_editor.h"
#include "file_dialog_locations.h"
#include "text_file_io.h"

EditorTabs::EditorTabs(QWidget *parent)
    : QTabWidget(parent)
{
    setDocumentMode(true);
    setMovable(true);
    setTabsClosable(false);
    tabBar()->setDrawBase(false);

    connect(this, &EditorTabs::tabCloseRequested, this, &EditorTabs::close_tab);
    connect(this, &EditorTabs::currentChanged, this, [this](int index) {
        Q_UNUSED(index);
        emit current_file_changed(current_file_path());
    });

    open_initial_document();
}

CodeEditor *EditorTabs::current_editor() const
{
    return qobject_cast<CodeEditor *>(currentWidget());
}

QString EditorTabs::current_file_path() const
{
    int index = currentIndex();

    if (index < 0) {
        return {};
    }

    return tabBar()->tabData(index).toString();
}

bool EditorTabs::open_file(const QString &path)
{
    QFileInfo file_info(path);

    if (!file_info.isFile()) {
        return false;
    }

    QString canonical_path = file_info.canonicalFilePath();

    for (int index = 0; index < count(); ++index) {
        if (tabBar()->tabData(index).toString() == canonical_path) {
            setCurrentIndex(index);
            return true;
        }
    }

    if (TextFileIo::is_binary_file(canonical_path)) {
        QMessageBox::information(this, tr("Binary file skipped"),
                                 tr("The selected file appears to be binary."));
        return false;
    }

    QString error;
    std::optional<QString> contents =
        TextFileIo::read_utf8_file(canonical_path, &error);

    if (!contents.has_value()) {
        QMessageBox::warning(this, tr("Unable to open file"), error);
        return false;
    }

    auto *editor = new CodeEditor(this);
    editor->setPlainText(contents.value());
    editor->set_diagnostics({});
    editor->document()->setModified(false);
    add_editor_tab(editor, file_info.fileName(), canonical_path);

    return true;
}

void EditorTabs::open_text_copy(const QString &title, const QString &contents)
{
    auto *editor = new CodeEditor(this);
    editor->setPlainText(contents);
    editor->set_diagnostics({});
    editor->document()->setModified(false);
    add_editor_tab(editor, title);
}

void EditorTabs::new_file()
{
    auto *editor = new CodeEditor(this);
    editor->set_diagnostics({});
    add_editor_tab(editor, tr("untitled.plat"));
}

bool EditorTabs::save_current_file()
{
    int index = currentIndex();

    if (index < 0) {
        return true;
    }

    return save_editor_at_index(index, current_file_path().isEmpty());
}

bool EditorTabs::save_current_file_as(const QString &path)
{
    int index = currentIndex();
    auto *editor = current_editor();

    if (index < 0 || editor == nullptr || path.isEmpty()) {
        return false;
    }

    QString error;

    if (!TextFileIo::write_utf8_file(path, editor->toPlainText(), &error)) {
        QMessageBox::warning(this, tr("Unable to save file"), error);
        return false;
    }

    set_editor_path(index, path);
    editor->document()->setModified(false);
    update_editor_tab_title(editor);
    emit current_file_changed(current_file_path());
    return true;
}

bool EditorTabs::save_all_files()
{
    for (int index = 0; index < count(); ++index) {
        if (!save_editor_at_index(index, tabBar()->tabData(index).toString().isEmpty())) {
            return false;
        }
    }

    return true;
}

bool EditorTabs::close_current_editor()
{
    int index = currentIndex();

    if (index < 0) {
        return true;
    }

    if (!confirm_close_editor(index)) {
        return false;
    }

    QWidget *editor = widget(index);
    removeTab(index);
    editor->deleteLater();
    emit current_file_changed(current_file_path());
    return true;
}

void EditorTabs::undo()
{
    if (auto *editor = current_editor()) {
        editor->undo();
    }
}

void EditorTabs::redo()
{
    if (auto *editor = current_editor()) {
        editor->redo();
    }
}

void EditorTabs::cut()
{
    if (auto *editor = current_editor()) {
        editor->cut();
    }
}

void EditorTabs::copy()
{
    if (auto *editor = current_editor()) {
        editor->copy();
    }
}

void EditorTabs::paste()
{
    if (auto *editor = current_editor()) {
        editor->paste();
    }
}

bool EditorTabs::find_text(const QString &text, QTextDocument::FindFlags flags)
{
    auto *editor = current_editor();

    if (editor == nullptr || text.isEmpty()) {
        return false;
    }

    if (editor->find(text, flags)) {
        return true;
    }

    QTextCursor original_cursor = editor->textCursor();
    QTextCursor wrapped_cursor = original_cursor;

    if (flags.testFlag(QTextDocument::FindBackward)) {
        wrapped_cursor.movePosition(QTextCursor::End);
    } else {
        wrapped_cursor.movePosition(QTextCursor::Start);
    }

    editor->setTextCursor(wrapped_cursor);

    if (editor->find(text, flags)) {
        return true;
    }

    editor->setTextCursor(original_cursor);
    return false;
}

bool EditorTabs::replace_current_match(const QString &find_text,
                                       const QString &replace_text)
{
    auto *editor = current_editor();

    if (editor == nullptr || find_text.isEmpty()) {
        return false;
    }

    QTextCursor cursor = editor->textCursor();

    if (!cursor.hasSelection() || cursor.selectedText() != find_text) {
        if (!this->find_text(find_text)) {
            return false;
        }

        cursor = editor->textCursor();
    }

    cursor.insertText(replace_text);
    return true;
}

int EditorTabs::replace_all_matches(const QString &find_text,
                                    const QString &replace_text)
{
    auto *editor = current_editor();

    if (editor == nullptr || find_text.isEmpty()) {
        return 0;
    }

    QTextCursor cursor = editor->textCursor();
    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::Start);
    editor->setTextCursor(cursor);

    int replacements = 0;

    while (editor->find(find_text)) {
        QTextCursor match_cursor = editor->textCursor();
        match_cursor.insertText(replace_text);
        ++replacements;
    }

    cursor = editor->textCursor();
    cursor.endEditBlock();
    editor->setTextCursor(cursor);

    return replacements;
}

bool EditorTabs::format_current_document()
{
    auto *editor = current_editor();

    if (editor == nullptr) {
        return false;
    }

    QStringList lines = editor->toPlainText().split('\n');

    for (QString &line : lines) {
        while (!line.isEmpty() && line.back().isSpace()) {
            line.chop(1);
        }
    }

    QString formatted = lines.join('\n');

    if (!formatted.endsWith('\n')) {
        formatted.append('\n');
    }

    QTextCursor cursor = editor->textCursor();
    int cursor_position = cursor.position();
    editor->setPlainText(formatted);
    cursor.setPosition(std::min(cursor_position,
                                static_cast<int>(formatted.size())));
    editor->setTextCursor(cursor);

    return true;
}

void EditorTabs::apply_theme(ThemeManager::Theme theme)
{
    for (int index = 0; index < count(); ++index) {
        auto *editor = qobject_cast<CodeEditor *>(widget(index));

        if (editor != nullptr) {
            editor->apply_theme(theme);
        }
    }
}

void EditorTabs::close_tab(int index)
{
    if (index < 0 || index >= count()) {
        return;
    }

    if (!confirm_close_editor(index)) {
        return;
    }

    QWidget *editor = widget(index);
    removeTab(index);
    editor->deleteLater();
    emit current_file_changed(current_file_path());
}

void EditorTabs::add_editor_tab(CodeEditor *editor, const QString &title,
                                const QString &path)
{
    editor->setProperty("base_tab_title", title);
    editor->document()->setModified(false);

    connect(editor->document(), &QTextDocument::modificationChanged,
            this, [this, editor](bool modified) {
                Q_UNUSED(modified);
                update_editor_tab_title(editor);
            });

    int index = addTab(editor, title);
    tabBar()->setTabData(index, path);
    setTabToolTip(index, path);
    install_close_button(index);
    setCurrentIndex(index);
    update_editor_tab_title(editor);
}

bool EditorTabs::save_editor_at_index(int index, bool prompt_for_path)
{
    auto *editor = qobject_cast<CodeEditor *>(widget(index));

    if (editor == nullptr) {
        return false;
    }

    QString path = tabBar()->tabData(index).toString();
    bool chose_path_in_dialog = false;

    if (prompt_for_path || path.isEmpty()) {
        QString file_name = editor->property("base_tab_title").toString();

        path = QFileDialog::getSaveFileName(
            this,
            tr("Save File"),
            FileDialogLocations::start_path(file_name),
            tr("platlang Files (*.plat);;All Files (*)"));

        if (path.isEmpty()) {
            return false;
        }

        chose_path_in_dialog = true;
    }

    QString error;

    if (!TextFileIo::write_utf8_file(path, editor->toPlainText(), &error)) {
        QMessageBox::warning(this, tr("Unable to save file"), error);
        return false;
    }

    set_editor_path(index, path);
    editor->document()->setModified(false);
    update_editor_tab_title(editor);

    if (chose_path_in_dialog) {
        FileDialogLocations::remember_file_path(path);
    }

    if (index == currentIndex()) {
        emit current_file_changed(current_file_path());
    }

    return true;
}

bool EditorTabs::confirm_close_editor(int index)
{
    auto *editor = qobject_cast<CodeEditor *>(widget(index));

    if (editor == nullptr || !editor->document()->isModified()) {
        return true;
    }

    QString title = editor->property("base_tab_title").toString();
    QMessageBox::StandardButton choice = QMessageBox::question(
        this,
        tr("Save changes?"),
        tr("Save changes to %1 before closing?").arg(title),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
        QMessageBox::Save);

    if (choice == QMessageBox::Cancel) {
        return false;
    }

    if (choice == QMessageBox::Discard) {
        return true;
    }

    return save_editor_at_index(index, tabBar()->tabData(index).toString().isEmpty());
}

void EditorTabs::set_editor_path(int index, const QString &path)
{
    QFileInfo file_info(path);
    QString canonical_path = file_info.absoluteFilePath();

    tabBar()->setTabData(index, canonical_path);
    setTabToolTip(index, canonical_path);

    if (auto *editor = qobject_cast<CodeEditor *>(widget(index))) {
        editor->setProperty("base_tab_title", file_info.fileName());
    }
}

void EditorTabs::update_editor_tab_title(CodeEditor *editor)
{
    int index = indexOf(editor);

    if (index < 0) {
        return;
    }

    QString title = editor->property("base_tab_title").toString();

    if (editor->document()->isModified()) {
        title = "● " + title;
    }

    setTabText(index, title);
}

void EditorTabs::install_close_button(int index)
{
    auto *button = new QToolButton(tabBar());
    button->setObjectName("tabCloseButton");
    button->setIcon(QIcon(":/assets/icons/fontawesome/solid/xmark.svg"));
    button->setIconSize(QSize(9, 9));
    button->setFixedSize(18, 18);
    button->setAutoRaise(true);
    button->setFocusPolicy(Qt::NoFocus);
    button->setToolTip(tr("Close editor"));

    connect(button, &QToolButton::clicked, this, [this, button]() {
        int tab_index = -1;

        for (int index = 0; index < count(); ++index) {
            if (tabBar()->tabButton(index, QTabBar::RightSide) == button) {
                tab_index = index;
                break;
            }
        }

        if (tab_index >= 0) {
            close_tab(tab_index);
        }
    });

    tabBar()->setTabButton(index, QTabBar::RightSide, button);
}

void EditorTabs::open_initial_document()
{
    auto *editor = new CodeEditor(this);
    editor->set_diagnostics({});
    add_editor_tab(editor, tr("untitled.plat"));
}
