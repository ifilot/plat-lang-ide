#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>

#include "compiler_toolchain.h"
#include "theme_manager.h"

class QActionGroup;
class QAction;
class BottomPanel;
class CodeAssistantPanel;
class EditorTabs;
class FileExplorerPanel;
class FindReplaceBar;
class QMenu;
class QNetworkAccessManager;
class TerminalPanel;
class TitleBar;

/**
 * Hosts the primary IDE workspace and top-level application chrome.
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    /**
     * Creates the main IDE window.
     *
     * @param startup_folder Optional folder to use as explorer root.
     * @param startup_message Optional startup message to show in the terminal.
     * @param parent Optional parent widget.
     */
    explicit MainWindow(const QString &startup_folder = {},
                        const QString &startup_message = {},
                        QWidget *parent = nullptr);

private:
    /**
     * Creates the top-level IDE menus and starter actions.
     */
    void setup_menu_bar();

private slots:
    /**
     * Opens a new untitled editor.
     */
    void new_file();

    /**
     * Opens a file from disk.
     */
    void open_file();

    /**
     * Selects a new root folder for the file explorer.
     */
    void open_folder();

    /**
     * Opens a recent file path from settings.
     *
     * @param path File path to reopen.
     */
    void open_recent_file(const QString &path);

    /**
     * Opens a bundled example as an unsaved editor document.
     *
     * @param resource_path Qt resource path for the example.
     * @param title Editor tab title.
     */
    void open_example(const QString &resource_path, const QString &title);

    /**
     * Saves the current editor.
     */
    void save_file();

    /**
     * Saves the current editor under a chosen path.
     */
    void save_file_as();

    /**
     * Saves all open editors.
     */
    void save_all_files();

    /**
     * Closes the current editor.
     */
    void close_editor();

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
     * Shows find controls for the active editor.
     */
    void show_find();

    /**
     * Shows replace controls for the active editor.
     */
    void show_replace();

    /**
     * Finds the next match from the find bar.
     */
    void find_next();

    /**
     * Finds the previous match from the find bar.
     */
    void find_previous();

    /**
     * Replaces the active match from the find bar.
     */
    void replace_current();

    /**
     * Replaces all matches from the find bar.
     */
    void replace_all();

    /**
     * Shows or hides the file explorer.
     */
    void set_file_explorer_visible(bool visible);

    /**
     * Shows or hides the code assistant.
     */
    void set_code_assistant_visible(bool visible);

    /**
     * Shows or hides the terminal bottom tab.
     */
    void set_terminal_visible(bool visible);

    /**
     * Shows or hides the problems bottom tab.
     */
    void set_problems_visible(bool visible);

    /**
     * Shows or hides the output bottom tab.
     */
    void set_output_visible(bool visible);

    /**
     * Updates terminal run controls for the active editor.
     *
     * @param path Current editor file path.
     */
    void update_run_target(const QString &path);

    /**
     * Runs the active platlang file through the configured compiler.
     */
    void run_current_file();

    /**
     * Stops the running platlang process.
     */
    void stop_current_process();

    /**
     * Parses the active file without executing it.
     */
    void build_current_file();

    /**
     * Builds and then runs the active file.
     */
    void build_and_run_current_file();

    /**
     * Prints the current compiler toolchain status to the terminal.
     */
    void show_compiler_status();

    /**
     * Lets the user select a compiler executable.
     */
    void configure_interpreter();

    /**
     * Checks whether the active interpreter matches the latest semver tag.
     */
    void check_compiler_updates();

    /**
     * Opens the command palette.
     */
    void show_command_palette();

    /**
     * Formats the active document.
     */
    void format_document();

    /**
     * Shows application settings.
     */
    void show_settings();

    /**
     * Opens project documentation.
     */
    void open_documentation();

    /**
     * Shows the application About dialog.
     */
    void show_about_dialog();

    /**
     * Applies the light application theme.
     */
    void use_light_theme();

    /**
     * Applies the dark application theme.
     */
    void use_dark_theme();

private:
    /**
     * Updates checked state for theme menu actions.
     *
     * @param theme Active theme.
     */
    void update_theme_actions(ThemeManager::Theme theme);

    /**
     * Rebuilds the recent files menu from settings.
     */
    void refresh_recent_files_menu();

    /**
     * Starts a parse-only build for the active file.
     *
     * @param run_after_success Whether to run after a successful build.
     */
    void start_build_current_file(bool run_after_success);

    /**
     * Executes a command palette command.
     *
     * @param command_id Stable command id.
     */
    void execute_command(const QString &command_id);

    /**
     * Checks the companion compiler repository for the newest semver tag.
     *
     * @param user_initiated Whether to print an explicit check-start message.
     */
    void check_latest_interpreter_version(bool user_initiated);

protected:
    /**
     * Keeps custom titlebar controls synced with window state changes.
     *
     * @param event Change event.
     */
    void changeEvent(QEvent *event) override;

private:
    FileExplorerPanel *file_explorer_;
    EditorTabs *editor_tabs_;
    FindReplaceBar *find_replace_bar_;
    BottomPanel *bottom_panel_;
    TerminalPanel *terminal_panel_;
    CodeAssistantPanel *code_assistant_;
    CompilerToolchain compiler_toolchain_;
    QNetworkAccessManager *interpreter_version_network_;
    bool interpreter_version_check_in_progress_;
    QAction *run_current_file_action_;
    QAction *stop_process_action_;
    QMenu *recent_files_menu_;
    QAction *file_explorer_action_;
    QAction *terminal_action_;
    QAction *problems_action_;
    QAction *output_action_;
    QAction *code_assistant_action_;
    QAction *light_theme_action_;
    QAction *dark_theme_action_;
    TitleBar *title_bar_;
    bool pending_run_after_build_;
};

#endif
