#include "main_window.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QDesktopServices>
#include <QDialog>
#include <QEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QIcon>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QPushButton>
#include <QRegularExpression>
#include <QSplitter>
#include <QTextDocument>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

#include "about_dialog.h"
#include "app_language.h"
#include "bottom_panel.h"
#include "command_palette_dialog.h"
#include "code_assistant_panel.h"
#include "editor_tabs.h"
#include "file_dialog_locations.h"
#include "file_explorer_panel.h"
#include "find_replace_bar.h"
#include "interpreter_settings.h"
#include "recent_files.h"
#include "settings_dialog.h"
#include "terminal_panel.h"
#include "title_bar.h"

namespace {
constexpr const char *latest_interpreter_release_url =
    "https://api.github.com/repos/ifilot/plat-lang/releases/latest";
constexpr const char *develop_interpreter_release_url =
    "https://api.github.com/repos/ifilot/plat-lang/releases/tags/develop-latest";

struct BundledExample {
    const char *title;
    const char *resource_path;
    const char *command_id;
};

constexpr BundledExample bundled_examples[] = {
    {QT_TRANSLATE_NOOP("MainWindow", "Hello"), ":/examples/hello.plat", "open_example_hello"},
    {QT_TRANSLATE_NOOP("MainWindow", "Conditions"), ":/examples/conditions.plat", "open_example_conditions"},
    {QT_TRANSLATE_NOOP("MainWindow", "Fibonacci"), ":/examples/fibonacci.plat", "open_example_fibonacci"},
    {QT_TRANSLATE_NOOP("MainWindow", "Loops"), ":/examples/loops.plat", "open_example_loops"},
    {QT_TRANSLATE_NOOP("MainWindow", "Problems"), ":/examples/problems.plat", "open_example_problems"},
    {QT_TRANSLATE_NOOP("MainWindow", "Tables"), ":/examples/tables.plat", "open_example_tables"},
};

bool is_semver_tag(const QString &tag)
{
    static const QRegularExpression semver_pattern(
        R"(^v?(0|[1-9]\d*)\.(0|[1-9]\d*)\.(0|[1-9]\d*)(?:-[0-9A-Za-z.-]+)?(?:\+[0-9A-Za-z.-]+)?$)");
    return semver_pattern.match(tag).hasMatch();
}

bool asset_matches_current_platform(const QString &asset_name)
{
    QString lower_name = asset_name.toLower();

#ifdef Q_OS_WIN
    return lower_name.contains("windows");
#else
    return lower_name.contains("linux") && !lower_name.endsWith(".exe");
#endif
}

QStringList interpreter_arguments(const QString &file_path,
                                  const QStringList &extra_arguments = {})
{
    QStringList arguments = extra_arguments;
    arguments.append(InterpreterSettings::arguments_for_preset(
        InterpreterSettings::load_argument_preset()));
    arguments.append(file_path);
    return arguments;
}
}

MainWindow::MainWindow(const QString &startup_folder,
                       const QString &startup_message,
                       QWidget *parent)
    : QMainWindow(parent),
      file_explorer_(new FileExplorerPanel(this)),
      editor_tabs_(new EditorTabs(this)),
      find_replace_bar_(new FindReplaceBar(this)),
      bottom_panel_(new BottomPanel(this)),
      terminal_panel_(bottom_panel_->terminal_panel()),
      code_assistant_(new CodeAssistantPanel(this)),
      interpreter_version_network_(new QNetworkAccessManager(this)),
      interpreter_version_check_in_progress_(false),
      run_current_file_action_(nullptr),
      stop_process_action_(nullptr),
      recent_files_menu_(nullptr),
      file_explorer_action_(nullptr),
      terminal_action_(nullptr),
      problems_action_(nullptr),
      output_action_(nullptr),
      code_assistant_action_(nullptr),
      light_theme_action_(nullptr),
      dark_theme_action_(nullptr),
      title_bar_(nullptr),
      pending_run_after_build_(false)
{
    setWindowFlag(Qt::FramelessWindowHint);
    setWindowTitle(tr("plat-lang IDE"));
    setWindowIcon(QIcon(":/logos/plat-lang-logo.svg"));
    resize(1440, 900);
    CompilerToolchain::Status toolchain_status = compiler_toolchain_.initialize();
    setup_menu_bar();

    auto *root = new QWidget(this);
    auto *root_layout = new QHBoxLayout(root);
    root_layout->setContentsMargins(0, 0, 0, 0);
    root_layout->setSpacing(0);

    auto *activity_bar = new QWidget(root);
    activity_bar->setObjectName("activityBar");
    activity_bar->setFixedWidth(44);

    auto *activity_layout = new QVBoxLayout(activity_bar);
    activity_layout->setContentsMargins(6, 8, 6, 8);
    activity_layout->setSpacing(6);

    auto *run_button = new QPushButton("▶", activity_bar);
    run_button->setObjectName("activityRunButton");
    run_button->setToolTip(tr("Run current file"));
    run_button->setFixedSize(32, 32);
    run_button->setFocusPolicy(Qt::NoFocus);
    activity_layout->addWidget(run_button);
    activity_layout->addStretch(1);

    auto *workspace = new QSplitter(Qt::Horizontal, root);
    auto *middle_column = new QSplitter(Qt::Vertical, workspace);
    auto *editor_area = new QWidget(middle_column);
    auto *editor_area_layout = new QVBoxLayout(editor_area);
    editor_area_layout->setContentsMargins(0, 0, 0, 0);
    editor_area_layout->setSpacing(0);
    editor_area_layout->addWidget(find_replace_bar_);
    editor_area_layout->addWidget(editor_tabs_, 1);
    workspace->setHandleWidth(7);
    middle_column->setHandleWidth(7);

    middle_column->addWidget(editor_area);
    middle_column->addWidget(bottom_panel_);
    middle_column->setStretchFactor(0, 4);
    middle_column->setStretchFactor(1, 1);
    middle_column->setSizes({650, 180});

    workspace->addWidget(file_explorer_);
    workspace->addWidget(middle_column);
    workspace->addWidget(code_assistant_);
    workspace->setStretchFactor(0, 1);
    workspace->setStretchFactor(1, 4);
    workspace->setStretchFactor(2, 1);
    workspace->setSizes({260, 900, 280});

    root_layout->addWidget(activity_bar);
    root_layout->addWidget(workspace, 1);

    if (!startup_folder.isEmpty()) {
        file_explorer_->set_root_directory(startup_folder);
    } else {
        file_explorer_->set_root_directory(FileDialogLocations::dialog_directory());
    }

    connect(file_explorer_, &FileExplorerPanel::file_open_requested,
            this, [this](const QString &path) {
                if (editor_tabs_->open_file(path)) {
                    RecentFiles::remember_file(path);
                    refresh_recent_files_menu();
                }
            });
    connect(editor_tabs_, &EditorTabs::current_file_changed,
            this, &MainWindow::update_run_target);
    connect(terminal_panel_, &TerminalPanel::run_requested,
            this, &MainWindow::run_current_file);
    connect(terminal_panel_, &TerminalPanel::process_running_changed,
            this, [this](bool running) {
                if (stop_process_action_ != nullptr) {
                    stop_process_action_->setEnabled(running);
                }
            });
    connect(terminal_panel_, &TerminalPanel::process_finished,
            this, [this](int exit_code, QProcess::ExitStatus exit_status) {
                if (!pending_run_after_build_) {
                    return;
                }

                pending_run_after_build_ = false;

                if (exit_status == QProcess::NormalExit && exit_code == 0) {
                    terminal_panel_->show_message(
                        tr("Build succeeded. Running current file."));
                    run_current_file();
                    return;
                }

                terminal_panel_->show_message(tr("Build failed. Run was skipped."));
            });
    connect(run_button, &QPushButton::clicked,
            this, &MainWindow::run_current_file);
    connect(find_replace_bar_, &FindReplaceBar::find_next_requested,
            this, &MainWindow::find_next);
    connect(find_replace_bar_, &FindReplaceBar::find_previous_requested,
            this, &MainWindow::find_previous);
    connect(find_replace_bar_, &FindReplaceBar::replace_requested,
            this, &MainWindow::replace_current);
    connect(find_replace_bar_, &FindReplaceBar::replace_all_requested,
            this, &MainWindow::replace_all);

    setCentralWidget(root);
    update_run_target(editor_tabs_->current_file_path());
    terminal_panel_->show_message(toolchain_status.message);
    bottom_panel_->append_output_message(toolchain_status.message);
    terminal_panel_->show_message(tr("Toolchain storage: ")
                                  + toolchain_status.storage_root);
    bottom_panel_->append_output_message(tr("Toolchain storage: ")
                                         + toolchain_status.storage_root);

    if (!startup_message.isEmpty()) {
        terminal_panel_->show_message(startup_message);
        bottom_panel_->append_output_message(startup_message);
    }

    if (!startup_folder.isEmpty()) {
        terminal_panel_->show_message(tr("Explorer root: ")
                                      + file_explorer_->root_directory());
        bottom_panel_->append_output_message(tr("Explorer root: ")
                                             + file_explorer_->root_directory());
    }

    QTimer::singleShot(0, this, [this]() {
        check_latest_interpreter_version(false);
    });
}

void MainWindow::setup_menu_bar()
{
    if (title_bar_ != nullptr) {
        title_bar_->deleteLater();
        title_bar_ = nullptr;
    }

    menuBar()->clear();

    auto add_scaffold_action = [](QMenu *menu, const QString &text,
                                  const QKeySequence &shortcut = QKeySequence()) {
        QAction *action = shortcut.isEmpty()
                              ? menu->addAction(text)
                              : menu->addAction(text, shortcut);
        action->setEnabled(false);
        return action;
    };

    auto *file_menu = menuBar()->addMenu(tr("&File"));
    file_menu->addAction(tr("&New File"), QKeySequence::New,
                         this, &MainWindow::new_file);
    file_menu->addAction(tr("&Open File..."), QKeySequence::Open,
                         this, &MainWindow::open_file);
    file_menu->addAction(tr("Open &Folder..."), this, &MainWindow::open_folder);
    recent_files_menu_ = file_menu->addMenu(tr("Open &Recent Files"));
    refresh_recent_files_menu();
    auto *examples_menu = file_menu->addMenu(tr("Open E&xample"));

    for (const BundledExample &example : bundled_examples) {
        examples_menu->addAction(tr(example.title), this, [this, example]() {
            open_example(example.resource_path,
                         QString("%1.plat").arg(tr(example.title)).toLower());
        });
    }

    file_menu->addSeparator();
    file_menu->addAction(tr("&Save"), QKeySequence::Save,
                         this, &MainWindow::save_file);
    file_menu->addAction(tr("Save &As..."), QKeySequence::SaveAs,
                         this, &MainWindow::save_file_as);
    file_menu->addAction(tr("Save A&ll"), this, &MainWindow::save_all_files);
    file_menu->addSeparator();
    file_menu->addAction(tr("&Close Editor"), QKeySequence::Close,
                         this, &MainWindow::close_editor);
    file_menu->addAction(tr("E&xit"), QKeySequence::Quit,
                         qApp, &QApplication::closeAllWindows);

    auto *edit_menu = menuBar()->addMenu(tr("&Edit"));
    edit_menu->addAction(tr("&Undo"), QKeySequence::Undo,
                         this, &MainWindow::undo);
    edit_menu->addAction(tr("&Redo"), QKeySequence::Redo,
                         this, &MainWindow::redo);
    edit_menu->addSeparator();
    edit_menu->addAction(tr("Cu&t"), QKeySequence::Cut,
                         this, &MainWindow::cut);
    edit_menu->addAction(tr("&Copy"), QKeySequence::Copy,
                         this, &MainWindow::copy);
    edit_menu->addAction(tr("&Paste"), QKeySequence::Paste,
                         this, &MainWindow::paste);
    edit_menu->addSeparator();
    edit_menu->addAction(tr("&Find..."), QKeySequence::Find,
                         this, &MainWindow::show_find);
    edit_menu->addAction(tr("Find &Next"), QKeySequence::FindNext,
                         this, &MainWindow::find_next);
    edit_menu->addAction(tr("Find &Previous"), QKeySequence::FindPrevious,
                         this, &MainWindow::find_previous);
    edit_menu->addAction(tr("&Replace..."), QKeySequence(Qt::CTRL | Qt::Key_H),
                         this, &MainWindow::show_replace);

    auto *view_menu = menuBar()->addMenu(tr("&View"));
    file_explorer_action_ = view_menu->addAction(tr("File &Explorer"),
                                                 this,
                                                 &MainWindow::set_file_explorer_visible);
    file_explorer_action_->setCheckable(true);
    file_explorer_action_->setChecked(!file_explorer_->isHidden());

    terminal_action_ = view_menu->addAction(tr("&Terminal"),
                                            this,
                                            &MainWindow::set_terminal_visible);
    terminal_action_->setCheckable(true);
    terminal_action_->setChecked(bottom_panel_->is_tab_visible(BottomPanel::Tab::Terminal));

    code_assistant_action_ = view_menu->addAction(tr("Code &Assistant"),
                                                  this,
                                                  &MainWindow::set_code_assistant_visible);
    code_assistant_action_->setCheckable(true);
    code_assistant_action_->setChecked(!code_assistant_->isHidden());
    view_menu->addSeparator();
    problems_action_ = view_menu->addAction(tr("&Problems"),
                                            this,
                                            &MainWindow::set_problems_visible);
    problems_action_->setCheckable(true);
    problems_action_->setChecked(bottom_panel_->is_tab_visible(BottomPanel::Tab::Problems));

    output_action_ = view_menu->addAction(tr("&Output"),
                                          this,
                                          &MainWindow::set_output_visible);
    output_action_->setCheckable(true);
    output_action_->setChecked(bottom_panel_->is_tab_visible(BottomPanel::Tab::Output));
    view_menu->addSeparator();

    auto *theme_menu = view_menu->addMenu(tr("&Theme"));
    auto *theme_group = new QActionGroup(this);
    theme_group->setExclusive(true);

    light_theme_action_ = theme_menu->addAction(tr("&Light"), this,
                                                &MainWindow::use_light_theme);
    light_theme_action_->setCheckable(true);
    theme_group->addAction(light_theme_action_);

    dark_theme_action_ = theme_menu->addAction(tr("&Dark"), this,
                                               &MainWindow::use_dark_theme);
    dark_theme_action_->setCheckable(true);
    theme_group->addAction(dark_theme_action_);
    update_theme_actions(ThemeManager::load_theme());

    auto *run_menu = menuBar()->addMenu(tr("&Run"));
    run_current_file_action_ = run_menu->addAction(
        tr("&Run Current File"), QKeySequence(Qt::CTRL | Qt::Key_R),
        this, &MainWindow::run_current_file);
    stop_process_action_ = run_menu->addAction(tr("&Stop"),
                                               QKeySequence(Qt::SHIFT | Qt::Key_F5),
                                               this,
                                               &MainWindow::stop_current_process);
    stop_process_action_->setEnabled(false);
    run_menu->addSeparator();
    run_menu->addAction(tr("&Build"), QKeySequence(Qt::CTRL | Qt::Key_B),
                        this, &MainWindow::build_current_file);
    run_menu->addAction(tr("Build && Run"),
                        QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_R),
                        this, &MainWindow::build_and_run_current_file);
    run_menu->addAction(tr("Compiler &Status"), this,
                        &MainWindow::show_compiler_status);
    run_menu->addAction(tr("Configure &Interpreter..."),
                        this, &MainWindow::configure_interpreter);
    run_menu->addAction(tr("Check for Compiler &Updates..."),
                        this, &MainWindow::check_compiler_updates);

    auto *tools_menu = menuBar()->addMenu(tr("&Tools"));
    tools_menu->addAction(tr("&Command Palette..."),
                          QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_P),
                          this, &MainWindow::show_command_palette);
    tools_menu->addAction(tr("&Format Document"),
                          QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_I),
                          this, &MainWindow::format_document);
    tools_menu->addAction(tr("&Settings..."),
                          QKeySequence(Qt::CTRL | Qt::Key_Comma),
                          this, &MainWindow::show_settings);

    auto *help_menu = menuBar()->addMenu(tr("&Help"));
    help_menu->addAction(tr("&Documentation"), QKeySequence::HelpContents,
                         this, &MainWindow::open_documentation);
    help_menu->addAction(tr("&About plat-lang IDE"), this,
                         &MainWindow::show_about_dialog);

    title_bar_ = new TitleBar(menuBar(), this);
    title_bar_->set_title(windowTitle());
    title_bar_->apply_theme(ThemeManager::load_theme());
    setMenuWidget(title_bar_);
}

void MainWindow::new_file()
{
    editor_tabs_->new_file();
}

void MainWindow::open_file()
{
    QString path = QFileDialog::getOpenFileName(
        this,
        tr("Open File"),
        FileDialogLocations::start_path(),
        tr("platlang Files (*.plat);;Text Files (*.txt *.md);;All Files (*)"));

    if (!path.isEmpty() && editor_tabs_->open_file(path)) {
        FileDialogLocations::remember_file_path(path);
        RecentFiles::remember_file(path);
        refresh_recent_files_menu();
    }
}

void MainWindow::open_folder()
{
    QString directory = QFileDialog::getExistingDirectory(
        this,
        tr("Open Folder"),
        FileDialogLocations::start_path(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (directory.isEmpty()) {
        return;
    }

    if (file_explorer_->set_root_directory(directory)) {
        FileDialogLocations::remember_directory(directory);
        terminal_panel_->show_message(tr("Explorer root: ") + directory);
        bottom_panel_->append_output_message(tr("Explorer root: ") + directory);
    }
}

void MainWindow::open_recent_file(const QString &path)
{
    if (editor_tabs_->open_file(path)) {
        RecentFiles::remember_file(path);
        refresh_recent_files_menu();
        return;
    }

    RecentFiles::forget_file(path);
    refresh_recent_files_menu();
    QMessageBox::warning(this, tr("Unable to open recent file"),
                         tr("Could not open %1.").arg(path));
}

void MainWindow::open_example(const QString &resource_path, const QString &title)
{
    QFile file(resource_path);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Unable to open example"),
                             tr("Could not read bundled example %1.").arg(title));
        return;
    }

    editor_tabs_->open_text_copy(title, QString::fromUtf8(file.readAll()));
}

void MainWindow::save_file()
{
    if (editor_tabs_->save_current_file()) {
        QString path = editor_tabs_->current_file_path();

        if (!path.isEmpty()) {
            RecentFiles::remember_file(path);
            refresh_recent_files_menu();
        }
    }
}

void MainWindow::save_file_as()
{
    QString file_name = QFileInfo(editor_tabs_->current_file_path()).fileName();

    if (file_name.isEmpty()) {
        file_name = tr("untitled.plat");
    }

    QString path = QFileDialog::getSaveFileName(
        this,
        tr("Save File As"),
        FileDialogLocations::start_path(file_name),
        tr("platlang Files (*.plat);;Text Files (*.txt *.md);;All Files (*)"));

    if (!path.isEmpty() && editor_tabs_->save_current_file_as(path)) {
        FileDialogLocations::remember_file_path(path);
        RecentFiles::remember_file(path);
        refresh_recent_files_menu();
    }
}

void MainWindow::save_all_files()
{
    if (editor_tabs_->save_all_files()) {
        QString path = editor_tabs_->current_file_path();

        if (!path.isEmpty()) {
            RecentFiles::remember_file(path);
            refresh_recent_files_menu();
        }
    }
}

void MainWindow::close_editor()
{
    editor_tabs_->close_current_editor();
}

void MainWindow::undo()
{
    editor_tabs_->undo();
}

void MainWindow::redo()
{
    editor_tabs_->redo();
}

void MainWindow::cut()
{
    editor_tabs_->cut();
}

void MainWindow::copy()
{
    editor_tabs_->copy();
}

void MainWindow::paste()
{
    editor_tabs_->paste();
}

void MainWindow::show_find()
{
    find_replace_bar_->show_find();
}

void MainWindow::show_replace()
{
    find_replace_bar_->show_replace();
}

void MainWindow::find_next()
{
    if (find_replace_bar_->find_text().isEmpty()) {
        find_replace_bar_->show_find();
        return;
    }

    editor_tabs_->find_text(find_replace_bar_->find_text());
}

void MainWindow::find_previous()
{
    if (find_replace_bar_->find_text().isEmpty()) {
        find_replace_bar_->show_find();
        return;
    }

    editor_tabs_->find_text(find_replace_bar_->find_text(),
                            QTextDocument::FindBackward);
}

void MainWindow::replace_current()
{
    if (find_replace_bar_->find_text().isEmpty()) {
        find_replace_bar_->show_replace();
        return;
    }

    editor_tabs_->replace_current_match(find_replace_bar_->find_text(),
                                        find_replace_bar_->replace_text());
}

void MainWindow::replace_all()
{
    if (find_replace_bar_->find_text().isEmpty()) {
        find_replace_bar_->show_replace();
        return;
    }

    int count = editor_tabs_->replace_all_matches(
        find_replace_bar_->find_text(),
        find_replace_bar_->replace_text());

    terminal_panel_->show_message(tr("Replaced %n occurrence(s).", nullptr, count)
                                  .arg(count));
    bottom_panel_->append_output_message(tr("Replaced %n occurrence(s).", nullptr, count)
                                         .arg(count));
}

void MainWindow::set_file_explorer_visible(bool visible)
{
    file_explorer_->setVisible(visible);
}

void MainWindow::set_code_assistant_visible(bool visible)
{
    code_assistant_->setVisible(visible);
}

void MainWindow::set_terminal_visible(bool visible)
{
    bottom_panel_->set_tab_visible(BottomPanel::Tab::Terminal, visible);
}

void MainWindow::set_problems_visible(bool visible)
{
    bottom_panel_->set_tab_visible(BottomPanel::Tab::Problems, visible);
}

void MainWindow::set_output_visible(bool visible)
{
    bottom_panel_->set_tab_visible(BottomPanel::Tab::Output, visible);
}

void MainWindow::update_run_target(const QString &path)
{
    bool can_run = !path.isEmpty() && QFileInfo(path).suffix() == "plat";
    terminal_panel_->set_run_target(path, can_run);
}

void MainWindow::run_current_file()
{
    QString file_path = editor_tabs_->current_file_path();

    if (file_path.isEmpty()) {
        terminal_panel_->show_message(
            tr("Save or open a .plat file before running it."));
        return;
    }

    if (QFileInfo(file_path).suffix() != "plat") {
        terminal_panel_->show_message(tr("Only .plat files can be run."));
        return;
    }

    CompilerToolchain::Status toolchain_status = compiler_toolchain_.status();

    if (!toolchain_status.available) {
        terminal_panel_->show_message(toolchain_status.message);
        terminal_panel_->show_message(tr("Toolchain storage: ")
                                      + toolchain_status.storage_root);
        return;
    }

    terminal_panel_->start_process(toolchain_status.compiler_path,
                                   interpreter_arguments(file_path),
                                   QFileInfo(file_path).absolutePath());
}

void MainWindow::stop_current_process()
{
    terminal_panel_->stop_process();
}

void MainWindow::build_current_file()
{
    start_build_current_file(false);
}

void MainWindow::build_and_run_current_file()
{
    start_build_current_file(true);
}

void MainWindow::start_build_current_file(bool run_after_success)
{
    pending_run_after_build_ = run_after_success;
    QString file_path = editor_tabs_->current_file_path();

    if (file_path.isEmpty()) {
        pending_run_after_build_ = false;
        terminal_panel_->show_message(
            tr("Save or open a .plat file before building it."));
        return;
    }

    if (QFileInfo(file_path).suffix() != "plat") {
        pending_run_after_build_ = false;
        terminal_panel_->show_message(tr("Only .plat files can be built."));
        return;
    }

    if (!editor_tabs_->save_current_file()) {
        pending_run_after_build_ = false;
        terminal_panel_->show_message(
            tr("Build cancelled because the file was not saved."));
        return;
    }

    CompilerToolchain::Status toolchain_status = compiler_toolchain_.status();

    if (!toolchain_status.available) {
        pending_run_after_build_ = false;
        terminal_panel_->show_message(toolchain_status.message);
        terminal_panel_->show_message(tr("Toolchain storage: ")
                                      + toolchain_status.storage_root);
        return;
    }

    terminal_panel_->show_message(
        tr("Building %1 with --ast.").arg(QFileInfo(file_path).fileName()));
    terminal_panel_->start_process(toolchain_status.compiler_path,
                                   interpreter_arguments(file_path, {"--ast"}),
                                   QFileInfo(file_path).absolutePath());

    if (!terminal_panel_->is_process_running()) {
        pending_run_after_build_ = false;
    }
}

void MainWindow::show_compiler_status()
{
    CompilerToolchain::Status toolchain_status = compiler_toolchain_.status();
    terminal_panel_->show_message(toolchain_status.message);
    bottom_panel_->append_output_message(toolchain_status.message);
    terminal_panel_->show_message(tr("Toolchain storage: ")
                                  + toolchain_status.storage_root);
    bottom_panel_->append_output_message(tr("Toolchain storage: ")
                                         + toolchain_status.storage_root);

    if (!toolchain_status.compiler_path.isEmpty()) {
        terminal_panel_->show_message(tr("Compiler path: ")
                                      + toolchain_status.compiler_path);
        bottom_panel_->append_output_message(tr("Compiler path: ")
                                             + toolchain_status.compiler_path);
    }
}

void MainWindow::configure_interpreter()
{
    QString path = QFileDialog::getOpenFileName(
        this,
        tr("Select platlang compiler"),
        FileDialogLocations::start_path(),
#ifdef Q_OS_WIN
        tr("Executables (*.exe);;All Files (*)")
#else
        tr("Executables (*)")
#endif
    );

    if (path.isEmpty()) {
        return;
    }

    FileDialogLocations::remember_file_path(path);
    CompilerToolchain::Status toolchain_status =
        compiler_toolchain_.install_compiler(path);
    terminal_panel_->show_message(toolchain_status.message);
    terminal_panel_->show_message(tr("Toolchain storage: ")
                                  + toolchain_status.storage_root);
    bottom_panel_->append_output_message(toolchain_status.message);
}

void MainWindow::check_compiler_updates()
{
    CompilerToolchain::Status toolchain_status = compiler_toolchain_.initialize();
    terminal_panel_->show_message(
        tr("Checked local development compiler for updates."));
    terminal_panel_->show_message(toolchain_status.message);
    bottom_panel_->append_output_message(
        tr("Checked local development compiler for updates."));
    bottom_panel_->append_output_message(toolchain_status.message);
    check_latest_interpreter_version(true);
}

void MainWindow::check_latest_interpreter_version(bool user_initiated)
{
    auto report = [this](const QString &message) {
        terminal_panel_->show_message(message);
        bottom_panel_->append_output_message(message);
    };

    if (interpreter_version_check_in_progress_) {
        if (user_initiated) {
            report(tr("Interpreter release check is already running."));
        }

        return;
    }

    interpreter_version_check_in_progress_ = true;
    CompilerToolchain::UpdateChannel update_channel =
        CompilerToolchain::load_update_channel();

    if (user_initiated
        && update_channel == CompilerToolchain::UpdateChannel::StableRelease) {
        report(tr("Checking latest platlang interpreter release."));
    }

    if (update_channel == CompilerToolchain::UpdateChannel::DevelopBranch) {
        if (user_initiated) {
            report(tr("Checking latest platlang develop prerelease."));
        }

        QNetworkRequest request{QUrl(develop_interpreter_release_url)};
        request.setRawHeader("Accept", "application/vnd.github+json");
        request.setRawHeader("User-Agent", "plat-lang-ide");

        QNetworkReply *reply = interpreter_version_network_->get(request);

        connect(reply, &QNetworkReply::finished, this,
                [this, reply, report]() {
                    reply->deleteLater();

                    if (reply->error() != QNetworkReply::NoError) {
                        interpreter_version_check_in_progress_ = false;
                        report(tr("Could not check latest platlang develop prerelease: ")
                               + reply->errorString());
                        return;
                    }

                    QJsonParseError parse_error;
                    QJsonDocument document = QJsonDocument::fromJson(
                        reply->readAll(), &parse_error);

                    if (parse_error.error != QJsonParseError::NoError
                        || !document.isObject()) {
                        interpreter_version_check_in_progress_ = false;
                        report(tr("Could not read latest platlang develop prerelease."));
                        return;
                    }

                    QJsonObject release = document.object();
                    QString tag = release.value("tag_name").toString();
                    QString sha = release.value("target_commitish").toString();
                    QString release_url = release.value("html_url").toString(
                        "https://github.com/ifilot/plat-lang/releases/tag/develop-latest");

                    if (tag != "develop-latest"
                        || !release.value("prerelease").toBool()
                        || sha.isEmpty()) {
                        interpreter_version_check_in_progress_ = false;
                        report(tr("Latest platlang develop prerelease metadata is incomplete."));
                        return;
                    }

                    QString version = CompilerToolchain::develop_version(sha);
                    CompilerToolchain::Status toolchain_status =
                        compiler_toolchain_.status();

                    if (toolchain_status.available
                        && toolchain_status.active_version == version) {
                        interpreter_version_check_in_progress_ = false;
                        report(tr("platlang develop interpreter is up to date (%1).")
                                   .arg(sha.left(12)));
                        return;
                    }

                    QString download_url;
                    const QJsonArray assets = release.value("assets").toArray();

                    for (const QJsonValue &asset_value : assets) {
                        QJsonObject asset = asset_value.toObject();
                        QString asset_name = asset.value("name").toString();

                        if (asset_matches_current_platform(asset_name)) {
                            download_url =
                                asset.value("browser_download_url").toString();
                            break;
                        }
                    }

                    if (download_url.isEmpty()) {
                        interpreter_version_check_in_progress_ = false;
                        report(tr("No platlang develop prerelease asset is available for this platform in %1.")
                               .arg(sha.left(12)));
                        QDesktopServices::openUrl(QUrl(release_url));
                        return;
                    }

                    QString current_version =
                        toolchain_status.active_version.isEmpty()
                            ? tr("none")
                            : toolchain_status.active_version;
                    QString prompt_text =
                        tr("Latest platlang develop prerelease: %1\n"
                           "Installed interpreter: %2\n\n"
                           "Download and install the latest experimental interpreter?")
                            .arg(sha.left(12), current_version);

                    QMessageBox prompt(this);
                    prompt.setWindowTitle(
                        tr("Download platlang Develop Interpreter"));
                    prompt.setText(prompt_text);
                    prompt.setIcon(QMessageBox::Information);
                    QPushButton *download_button =
                        prompt.addButton(tr("Download"), QMessageBox::AcceptRole);
                    QPushButton *release_button =
                        prompt.addButton(tr("Open Release"),
                                         QMessageBox::ActionRole);
                    prompt.addButton(tr("Later"), QMessageBox::RejectRole);
                    prompt.setDefaultButton(download_button);
                    prompt.exec();

                    if (prompt.clickedButton() == release_button) {
                        interpreter_version_check_in_progress_ = false;
                        QDesktopServices::openUrl(QUrl(release_url));
                        report(tr("Opened platlang develop prerelease %1.")
                                   .arg(sha.left(12)));
                        return;
                    }

                    if (prompt.clickedButton() != download_button) {
                        interpreter_version_check_in_progress_ = false;
                        report(tr("Skipped platlang develop interpreter download for %1.")
                                   .arg(sha.left(12)));
                        return;
                    }

                    report(tr("Downloading platlang develop interpreter %1.")
                               .arg(sha.left(12)));

                    QNetworkRequest download_request{QUrl(download_url)};
                    download_request.setRawHeader("Accept",
                                                  "application/octet-stream");
                    download_request.setRawHeader("User-Agent", "plat-lang-ide");

                    QNetworkReply *download_reply =
                        interpreter_version_network_->get(download_request);

                    connect(download_reply, &QNetworkReply::finished,
                            this,
                            [this, download_reply, report, sha, version]() {
                                download_reply->deleteLater();
                                interpreter_version_check_in_progress_ = false;

                                if (download_reply->error()
                                    != QNetworkReply::NoError) {
                                    report(tr("Could not download platlang develop interpreter %1: ")
                                               .arg(sha.left(12))
                                           + download_reply->errorString());
                                    return;
                                }

                                CompilerToolchain::Status installed_status =
                                    compiler_toolchain_.install_compiler_data(
                                        download_reply->readAll(), version);
                                report(installed_status.message);

                                if (installed_status.available
                                    && installed_status.active_version == version) {
                                    report(tr("Installed latest platlang develop interpreter %1.")
                                               .arg(sha.left(12)));
                                }
                            });
                });
        return;
    }

    QNetworkRequest request{QUrl(latest_interpreter_release_url)};
    request.setRawHeader("Accept", "application/vnd.github+json");
    request.setRawHeader("User-Agent", "plat-lang-ide");

    QNetworkReply *reply = interpreter_version_network_->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, report]() {
        reply->deleteLater();
        interpreter_version_check_in_progress_ = false;

        if (reply->error() != QNetworkReply::NoError) {
            report(tr("Could not check latest platlang interpreter release: ")
                   + reply->errorString());
            return;
        }

        QJsonParseError parse_error;
        QJsonDocument document = QJsonDocument::fromJson(reply->readAll(),
                                                         &parse_error);

        if (parse_error.error != QJsonParseError::NoError
            || !document.isObject()) {
            report(tr("Could not read latest platlang interpreter release."));
            return;
        }

        QJsonObject release = document.object();
        QString tag = release.value("tag_name").toString();

        if (!is_semver_tag(tag)) {
            report(tr("Latest platlang interpreter release does not use a semver tag."));
            return;
        }

        QString release_url = release.value("html_url").toString(
            "https://github.com/ifilot/plat-lang/releases");
        QString download_url;
        const QJsonArray assets = release.value("assets").toArray();

        for (const QJsonValue &asset_value : assets) {
            QJsonObject asset = asset_value.toObject();
            QString asset_name = asset.value("name").toString();

            if (asset_matches_current_platform(asset_name)) {
                download_url = asset.value("browser_download_url").toString();
                break;
            }
        }

        CompilerToolchain::Status toolchain_status = compiler_toolchain_.status();

        if (toolchain_status.available && toolchain_status.active_version == tag) {
            report(tr("platlang interpreter is up to date (%1).").arg(tag));
            return;
        }

        QString current_version = toolchain_status.active_version.isEmpty()
                                      ? tr("none")
                                      : toolchain_status.active_version;
        QString prompt_text = tr("Latest platlang interpreter: %1\n"
                                 "Installed interpreter: %2\n\n"
                                 "Download and install the latest interpreter?")
                                  .arg(tag, current_version);

        QMessageBox prompt(this);
        prompt.setWindowTitle(tr("Download platlang Interpreter"));
        prompt.setText(prompt_text);
        prompt.setIcon(QMessageBox::Information);
        QPushButton *download_button =
            prompt.addButton(tr("Download"), QMessageBox::AcceptRole);
        QPushButton *release_button =
            prompt.addButton(tr("Open Release"), QMessageBox::ActionRole);
        prompt.addButton(tr("Later"), QMessageBox::RejectRole);
        prompt.setDefaultButton(download_button);
        prompt.exec();

        if (prompt.clickedButton() == release_button) {
            QDesktopServices::openUrl(QUrl(release_url));
            report(tr("Opened platlang interpreter release %1.").arg(tag));
            return;
        }

        if (prompt.clickedButton() != download_button) {
            report(tr("Skipped platlang interpreter download for %1.").arg(tag));
            return;
        }

        if (download_url.isEmpty()) {
            report(tr("No platlang interpreter asset is available for this platform in %1.")
                   .arg(tag));
            QDesktopServices::openUrl(QUrl(release_url));
            return;
        }

        report(tr("Downloading platlang interpreter %1.").arg(tag));

        QNetworkRequest download_request{QUrl(download_url)};
        download_request.setRawHeader("Accept", "application/octet-stream");
        download_request.setRawHeader("User-Agent", "plat-lang-ide");

        QNetworkReply *download_reply =
            interpreter_version_network_->get(download_request);

        connect(download_reply, &QNetworkReply::finished, this,
                [this, download_reply, tag, report]() {
                    download_reply->deleteLater();

                    if (download_reply->error() != QNetworkReply::NoError) {
                        report(tr("Could not download platlang interpreter %1: ")
                                   .arg(tag)
                               + download_reply->errorString());
                        return;
                    }

                    CompilerToolchain::Status installed_status =
                        compiler_toolchain_.install_compiler_data(
                            download_reply->readAll(), tag);
                    report(installed_status.message);

                    if (installed_status.available
                        && installed_status.active_version == tag) {
                        report(tr("Installed latest platlang interpreter %1.")
                                   .arg(tag));
                    }
                });
    });
}

void MainWindow::show_command_palette()
{
    CommandPaletteDialog dialog(this);

    if (dialog.exec() == QDialog::Accepted) {
        execute_command(dialog.selected_command());
    }
}

void MainWindow::format_document()
{
    if (editor_tabs_->format_current_document()) {
        terminal_panel_->show_message(tr("Formatted current document."));
        bottom_panel_->append_output_message(tr("Formatted current document."));
    } else {
        terminal_panel_->show_message(tr("No open document to format."));
    }
}

void MainWindow::show_settings()
{
    SettingsDialog dialog(ThemeManager::load_theme(),
                          compiler_toolchain_.status(),
                          this);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    ThemeManager::Theme selected_theme = dialog.selected_theme();
    InterpreterSettings::save_argument_preset(
        dialog.selected_interpreter_argument_preset());
    CompilerToolchain::save_update_channel(
        dialog.selected_compiler_update_channel());
    AppLanguage::save_language(dialog.selected_language());
    AppLanguage::install_translator(*qApp);

    if (selected_theme == ThemeManager::Theme::Dark) {
        use_dark_theme();
    } else {
        use_light_theme();
    }
}

void MainWindow::open_documentation()
{
    QDesktopServices::openUrl(
        QUrl("https://github.com/ifilot/plat-lang-ide"));
}

void MainWindow::show_about_dialog()
{
    AboutDialog::show(this);
}

void MainWindow::use_light_theme()
{
    ThemeManager::apply_theme(*qApp, ThemeManager::Theme::Light);
    editor_tabs_->apply_theme(ThemeManager::Theme::Light);
    terminal_panel_->apply_theme();
    title_bar_->apply_theme(ThemeManager::Theme::Light);
    update_theme_actions(ThemeManager::Theme::Light);
}

void MainWindow::use_dark_theme()
{
    ThemeManager::apply_theme(*qApp, ThemeManager::Theme::Dark);
    editor_tabs_->apply_theme(ThemeManager::Theme::Dark);
    terminal_panel_->apply_theme();
    title_bar_->apply_theme(ThemeManager::Theme::Dark);
    update_theme_actions(ThemeManager::Theme::Dark);
}

void MainWindow::update_theme_actions(ThemeManager::Theme theme)
{
    if (light_theme_action_ != nullptr) {
        light_theme_action_->setChecked(theme == ThemeManager::Theme::Light);
    }

    if (dark_theme_action_ != nullptr) {
        dark_theme_action_->setChecked(theme == ThemeManager::Theme::Dark);
    }
}

void MainWindow::refresh_recent_files_menu()
{
    if (recent_files_menu_ == nullptr) {
        return;
    }

    recent_files_menu_->clear();
    QStringList recent_files = RecentFiles::files();

    if (recent_files.isEmpty()) {
        QAction *empty_action = recent_files_menu_->addAction(tr("No Recent Files"));
        empty_action->setEnabled(false);
        return;
    }

    for (const QString &path : recent_files) {
        QAction *action = recent_files_menu_->addAction(QFileInfo(path).fileName());
        action->setToolTip(path);
        connect(action, &QAction::triggered, this, [this, path]() {
            open_recent_file(path);
        });
    }
}

void MainWindow::execute_command(const QString &command_id)
{
    if (command_id == "new_file") {
        new_file();
    } else if (command_id == "open_file") {
        open_file();
    } else if (command_id == "open_folder") {
        open_folder();
    } else if (command_id.startsWith("open_example_")) {
        for (const BundledExample &example : bundled_examples) {
            if (command_id == example.command_id) {
                open_example(example.resource_path,
                             QString("%1.plat").arg(tr(example.title)).toLower());
                return;
            }
        }
    } else if (command_id == "save_file") {
        save_file();
    } else if (command_id == "run_current_file") {
        run_current_file();
    } else if (command_id == "build_current_file") {
        build_current_file();
    } else if (command_id == "build_and_run_current_file") {
        build_and_run_current_file();
    } else if (command_id == "toggle_file_explorer") {
        file_explorer_action_->trigger();
    } else if (command_id == "toggle_terminal") {
        terminal_action_->trigger();
    } else if (command_id == "toggle_problems") {
        problems_action_->trigger();
    } else if (command_id == "toggle_output") {
        output_action_->trigger();
    } else if (command_id == "toggle_code_assistant") {
        code_assistant_action_->trigger();
    } else if (command_id == "format_document") {
        format_document();
    } else if (command_id == "settings") {
        show_settings();
    } else if (command_id == "documentation") {
        open_documentation();
    } else if (command_id == "about") {
        show_about_dialog();
    }
}

void MainWindow::changeEvent(QEvent *event)
{
    QMainWindow::changeEvent(event);

    if (event->type() == QEvent::LanguageChange) {
        setWindowTitle(tr("plat-lang IDE"));
        setup_menu_bar();
        update_run_target(editor_tabs_->current_file_path());
        return;
    }

    if (event->type() == QEvent::WindowStateChange && title_bar_ != nullptr) {
        title_bar_->refresh_window_state();
    }
}
