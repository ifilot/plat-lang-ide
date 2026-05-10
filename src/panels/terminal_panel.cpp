#include "terminal_panel.h"

#include <QColor>
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
      run_target_label_(new QLabel("No runnable .plat file open.", this)),
      run_button_(new QPushButton("Run", this)),
      send_button_(new QPushButton("Send", this))
{
    output_->setReadOnly(true);
    output_->setLineWrapMode(QTextEdit::NoWrap);
    input_->setPlaceholderText("Input for the running program...");
    run_button_->setEnabled(false);
    append_terminal_text("Compiler-interpreter terminal ready.",
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
    process_->start(program, arguments);

    if (!process_->waitForStarted(3000)) {
        append_terminal_text("Failed to start compiler-interpreter: "
                             + process_->errorString(),
                             TerminalTextKind::MutedInfo);
        emit process_running_changed(false);
        return;
    }

    emit process_running_changed(true);
}

void TerminalPanel::set_run_target(const QString &path, bool can_run)
{
    if (path.isEmpty()) {
        run_button_->setEnabled(false);
        run_target_label_->setText("No runnable .plat file open.");
        run_target_label_->setToolTip({});
        return;
    }

    QFileInfo file_info(path);
    run_button_->setEnabled(can_run);
    run_target_label_->setText(can_run ? "Run target: " + file_info.fileName()
                                       : "Open a .plat file to run.");
    run_target_label_->setToolTip(path);
}

void TerminalPanel::show_message(const QString &text)
{
    append_terminal_text(text, TerminalTextKind::MutedInfo);
}

void TerminalPanel::stop_process()
{
    if (process_->state() == QProcess::NotRunning) {
        append_terminal_text("No compiler-interpreter process is running.",
                             TerminalTextKind::MutedInfo);
        return;
    }

    append_terminal_text("Stopping compiler-interpreter process.",
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
        append_terminal_text("No compiler-interpreter process is running.",
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
    QString status = exit_status == QProcess::NormalExit ? "exited" : "crashed";
    append_terminal_text(QString("Process %1 with code %2.")
                         .arg(status)
                         .arg(exit_code),
                         TerminalTextKind::MutedInfo);
    emit process_finished(exit_code, exit_status);
    emit process_running_changed(false);
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
