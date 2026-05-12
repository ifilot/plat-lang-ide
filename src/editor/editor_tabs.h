#ifndef EDITOR_TABS_H
#define EDITOR_TABS_H

#include <QTabWidget>
#include <QTextDocument>

#include "theme_manager.h"

class CodeEditor;

/**
 * Manages open editor documents as tabs.
 */
class EditorTabs : public QTabWidget {
    Q_OBJECT

public:
    /**
     * Creates the tabbed editor container.
     *
     * @param parent Optional parent widget.
     */
    explicit EditorTabs(QWidget *parent = nullptr);

    /**
     * Returns the currently active editor.
     *
     * @return Active editor, or nullptr when no editor is open.
     */
    CodeEditor *current_editor() const;

    /**
     * Returns the file path for the active editor.
     *
     * @return Current file path, or empty string for unsaved editors.
     */
    QString current_file_path() const;

    /**
     * Opens a text file in an editor tab.
     *
     * @param path Absolute or relative file path.
     * @return True when the file was opened, false when it was skipped.
     */
    bool open_file(const QString &path);

    /**
     * Opens text as an unsaved editor document.
     *
     * @param title Tab title to show.
     * @param contents Text contents.
     */
    void open_text_copy(const QString &title, const QString &contents);

    /**
     * Opens a new untitled editor.
     */
    void new_file();

    /**
     * Saves the current editor to its existing path.
     *
     * @return True when saved.
     */
    bool save_current_file();

    /**
     * Saves the current editor to a chosen path.
     *
     * @param path Destination path.
     * @return True when saved.
     */
    bool save_current_file_as(const QString &path);

    /**
     * Saves every open editor, prompting for untitled file paths.
     *
     * @return True when all editors were saved.
     */
    bool save_all_files();

    /**
     * Closes the current editor after checking for unsaved changes.
     *
     * @return True when the editor was closed or no editor was open.
     */
    bool close_current_editor();

    /**
     * Applies undo to the active editor.
     */
    void undo();

    /**
     * Applies redo to the active editor.
     */
    void redo();

    /**
     * Cuts selected text from the active editor.
     */
    void cut();

    /**
     * Copies selected text from the active editor.
     */
    void copy();

    /**
     * Pastes clipboard text into the active editor.
     */
    void paste();

    /**
     * Finds text in the active editor, wrapping around document edges.
     *
     * @param text Text to find.
     * @param flags QTextDocument search flags.
     * @return True when a match was found.
     */
    bool find_text(const QString &text,
                   QTextDocument::FindFlags flags = {});

    /**
     * Replaces the active match or finds and replaces the next match.
     *
     * @param find_text Text to search for.
     * @param replace_text Replacement text.
     * @return True when text was replaced.
     */
    bool replace_current_match(const QString &find_text,
                               const QString &replace_text);

    /**
     * Replaces every occurrence in the active editor.
     *
     * @param find_text Text to search for.
     * @param replace_text Replacement text.
     * @return Number of replacements performed.
     */
    int replace_all_matches(const QString &find_text,
                            const QString &replace_text);

    /**
     * Applies simple whitespace formatting to the active document.
     *
     * @return True when a document was formatted.
     */
    bool format_current_document();

    /**
     * Applies a theme to all open editors.
     *
     * @param theme Active application theme.
     */
    void apply_theme(ThemeManager::Theme theme);

signals:
    /**
     * Emitted when the active editor file changes.
     *
     * @param path Current editor file path, or empty for unsaved editors.
     */
    void current_file_changed(const QString &path);

private slots:
    /**
     * Closes an editor tab after checking for unsaved changes.
     *
     * @param index Tab index to close.
     */
    void close_tab(int index);

private:
    /**
     * Adds an editor tab and connects tab title state.
     *
     * @param editor Editor to add.
     * @param title Base tab title.
     * @param path File path, or empty for untitled editors.
     */
    void add_editor_tab(CodeEditor *editor, const QString &title,
                        const QString &path = {});

    /**
     * Saves an editor tab.
     *
     * @param index Tab index.
     * @param prompt_for_path Whether to prompt even when a path exists.
     * @return True when saved.
     */
    bool save_editor_at_index(int index, bool prompt_for_path);

    /**
     * Asks how to handle unsaved changes before closing.
     *
     * @param index Tab index.
     * @return True when closing may continue.
     */
    bool confirm_close_editor(int index);

    /**
     * Updates an editor tab file path and base title.
     *
     * @param index Tab index.
     * @param path File path.
     */
    void set_editor_path(int index, const QString &path);

    /**
     * Updates an editor tab title for saved/modified state.
     *
     * @param editor Editor whose tab should be updated.
     */
    void update_editor_tab_title(CodeEditor *editor);

    /**
     * Installs a flat close button on a tab.
     *
     * @param index Tab index.
     */
    void install_close_button(int index);

    /**
     * Opens the initial untitled document.
     */
    void open_initial_document();

};

#endif
