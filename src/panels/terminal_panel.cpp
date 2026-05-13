#include "terminal_panel.h"

#include <QColor>
#include <QDir>
#include <QEvent>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QVBoxLayout>

TerminalPanel::TerminalPanel(QWidget *parent)
    : QWidget(parent),
      process_(new QProcess(this)),
      output_(new QTextEdit(this)),
      input_(new QLineEdit(this)),
      run_target_label_(new QLabel(this)),
      run_button_(new QPushButton(this)),
      send_button_(new QPushButton(this)),
      current_run_target_can_run_(false)
{
    output_->setReadOnly(true);
    output_->setLineWrapMode(QTextEdit::NoWrap);
    retranslate_ui();
    run_button_->setEnabled(false);
    append_terminal_text(tr("Compiler-interpreter terminal ready."),
                         TerminalTextKind::MutedInfo);

    process_->setProcessChannelMode(QProcess::MergedChannels);

    connect(run_button_, &QPushButton::clicked,
            this, &TerminalPanel::run_requested);
    connect(send_button_, &QPushButton::clicked,
            this, &TerminalPanel::send_input);
    connect(input_, &QLineEdit::returnPressed,
            this, &TerminalPanel::send_input);
    connect(process_, &QProcess::readyReadStandardOutput,
            this, &TerminalPanel::read_process_output);
    connect(process_, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &TerminalPanel::handle_process_finished);

    auto *run_layout = new QHBoxLayout();
    run_layout->setContentsMargins(0, 0, 0, 0);
    run_layout->addWidget(run_target_label_, 1);
    run_layout->addWidget(run_button_);

    auto *input_layout = new QHBoxLayout();
    input_layout->setContentsMargins(0, 0, 0, 0);
    input_layout->addWidget(input_, 1);
    input_layout->addWidget(send_button_);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addLayout(run_layout);
    layout->addWidget(output_, 1);
    layout->addLayout(input_layout);
}

void TerminalPanel::start_process(const QString &program,
                                  const QStringList &arguments,
                                  const QString &working_directory)
{
    if (process_->state() != QProcess::NotRunning) {
        process_->kill();
        process_->waitForFinished(1000);
    }

    output_->clear();
    if (!working_directory.isEmpty()) {
        process_->setWorkingDirectory(working_directory);
    }
    append_terminal_text("> " + program + " " + arguments.join(' '),
                         TerminalTextKind::MutedInfo);
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    const QString compiler_directory = QFileInfo(program).absolutePath();

    if (!compiler_directory.isEmpty()) {
#ifdef Q_OS_WIN
        const QString library_path_name = QStringLiteral("PATH");
#elif defined(Q_OS_MACOS)
        const QString library_path_name = QStringLiteral("DYLD_LIBRARY_PATH");
#else
        const QString library_path_name = QStringLiteral("LD_LIBRARY_PATH");
#endif
        const QString existing_library_path =
            environment.value(library_path_name);
        QStringList library_paths;
        library_paths.append(compiler_directory);

        const QString package_library_directory =
            QDir(compiler_directory).absoluteFilePath("../lib");

        if (QFileInfo(package_library_directory).isDir()) {
            library_paths.append(QDir(package_library_directory).absolutePath());
        }

        if (!existing_library_path.isEmpty()) {
            library_paths.append(existing_library_path);
        }

        environment.insert(
            library_path_name,
            library_paths.join(QDir::listSeparator()));
        process_->setProcessEnvironment(environment);
    }

    process_->start(program, arguments);

    if (!process_->waitForStarted(3000)) {
        append_terminal_text(tr("Failed to start compiler-interpreter: ")
                             + process_->errorString(),
                             TerminalTextKind::MutedInfo);
        emit process_running_changed(false);
        return;
    }

    emit process_running_changed(true);
}

void TerminalPanel::set_run_target(const QString &path, bool can_run)
{
    current_run_target_path_ = path;
    current_run_target_can_run_ = can_run;

    if (path.isEmpty()) {
        run_button_->setEnabled(false);
        run_target_label_->setText(tr("No runnable .plat file open."));
        run_target_label_->setToolTip({});
        return;
    }

    QFileInfo file_info(path);
    run_button_->setEnabled(can_run);
    run_target_label_->setText(can_run ? tr("Run target: %1").arg(file_info.fileName())
                                       : tr("Open a .plat file to run."));
    run_target_label_->setToolTip(path);
}

void TerminalPanel::show_message(const QString &text)
{
    append_terminal_text(text, TerminalTextKind::MutedInfo);
}

void TerminalPanel::stop_process()
{
    if (process_->state() == QProcess::NotRunning) {
        append_terminal_text(tr("No compiler-interpreter process is running."),
                             TerminalTextKind::MutedInfo);
        return;
    }

    append_terminal_text(tr("Stopping compiler-interpreter process."),
                         TerminalTextKind::MutedInfo);
    process_->terminate();

    if (!process_->waitForFinished(1500)) {
        process_->kill();
    }
}

bool TerminalPanel::is_process_running() const
{
    return process_->state() != QProcess::NotRunning;
}

void TerminalPanel::send_input()
{
    QString text = input_->text();

    if (text.isEmpty()) {
        return;
    }

    append_terminal_text("< " + text, TerminalTextKind::MutedInfo);
    input_->clear();

    if (process_->state() == QProcess::Running) {
        process_->write(text.toUtf8());
        process_->write("\n");
    } else {
        append_terminal_text(tr("No compiler-interpreter process is running."),
                             TerminalTextKind::MutedInfo);
    }
}

void TerminalPanel::read_process_output()
{
    append_terminal_text(QString::fromUtf8(process_->readAllStandardOutput()),
                         TerminalTextKind::ProgramOutput);
}

void TerminalPanel::handle_process_finished(int exit_code,
                                            QProcess::ExitStatus exit_status)
{
    QString status = exit_status == QProcess::NormalExit ? tr("exited") : tr("crashed");
    append_terminal_text(tr("Process %1 with code %2.")
                         .arg(status)
                         .arg(exit_code),
                         TerminalTextKind::MutedInfo);
    emit process_finished(exit_code, exit_status);
    emit process_running_changed(false);
}

void TerminalPanel::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);

    if (event->type() == QEvent::LanguageChange) {
        retranslate_ui();
    }
}

void TerminalPanel::retranslate_ui()
{
    input_->setPlaceholderText(tr("Input for the running program..."));
    run_button_->setText(tr("Run"));
    send_button_->setText(tr("Send"));
    set_run_target(current_run_target_path_, current_run_target_can_run_);
}

void TerminalPanel::append_terminal_text(const QString &text,
                                         TerminalTextKind kind)
{
    QTextCharFormat format;
    format.setForeground(terminal_text_color(kind));

    output_->moveCursor(QTextCursor::End);
    output_->textCursor().insertText(text, format);

    if (!text.endsWith('\n')) {
        output_->textCursor().insertText("\n", format);
    }

    output_->moveCursor(QTextCursor::End);
}

QColor TerminalPanel::terminal_text_color(TerminalTextKind kind) const
{
    if (kind == TerminalTextKind::ProgramOutput) {
        return output_->palette().text().color();
    }

    return output_->palette().placeholderText().color();
}
