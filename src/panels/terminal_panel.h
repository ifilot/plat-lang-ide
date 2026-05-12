#ifndef TERMINAL_PANEL_H
#define TERMINAL_PANEL_H

#include <QProcess>
#include <QWidget>

class QColor;
class QEvent;
class QLineEdit;
class QLabel;
class QPushButton;
class QTextEdit;

/**
 * Provides interactive I/O with the compiler-interpreter process.
 */
class TerminalPanel : public QWidget {
    Q_OBJECT

public:
    /**
     * Creates the terminal panel.
     *
     * @param parent Optional parent widget.
     */
    explicit TerminalPanel(QWidget *parent = nullptr);

    /**
     * Starts the compiler-interpreter process.
     *
     * @param program Executable path.
     * @param arguments Program arguments.
     */
    void start_process(const QString &program,
                       const QStringList &arguments = {},
                       const QString &working_directory = {});

    /**
     * Updates the file shown as runnable from the terminal.
     *
     * @param path File path to run.
     * @param can_run Whether the file can be run.
     */
    void set_run_target(const QString &path, bool can_run);

    /**
     * Stops the running compiler-interpreter process.
     */
    void stop_process();

    /**
     * Returns whether a process is currently running.
     *
     * @return True when the terminal owns a live process.
     */
    bool is_process_running() const;

    /**
     * Shows an informational message in the terminal.
     *
     * @param text Message text.
     */
    void show_message(const QString &text);

signals:
    /**
     * Emitted when the user requests running the active file.
     */
    void run_requested();

    /**
     * Emitted when process running state changes.
     *
     * @param running Whether a process is running.
     */
    void process_running_changed(bool running);

    /**
     * Emitted when the running process exits.
     *
     * @param exit_code Process exit code.
     * @param exit_status Process exit status.
     */
    void process_finished(int exit_code, QProcess::ExitStatus exit_status);

private slots:
    /**
     * Sends the current input line to the running process.
     */
    void send_input();

    /**
     * Appends available process output to the terminal.
     */
    void read_process_output();

    /**
     * Updates the terminal when the process exits.
     *
     * @param exit_code Process exit code.
     * @param exit_status Process exit status.
     */
    void handle_process_finished(int exit_code, QProcess::ExitStatus exit_status);

private:
    enum class TerminalTextKind {
        ProgramOutput,
        MutedInfo
    };

    /**
     * Appends one terminal line with a lightweight prefix.
     *
     * @param text Text to append.
     * @param kind Visual style to use.
     */
    void append_terminal_text(const QString &text, TerminalTextKind kind);

    /**
     * Returns the text color for a terminal message kind.
     *
     * @param kind Visual style to use.
     * @return Text color.
     */
    QColor terminal_text_color(TerminalTextKind kind) const;

    void changeEvent(QEvent *event) override;
    void retranslate_ui();

    QProcess *process_;
    QTextEdit *output_;
    QLineEdit *input_;
    QLabel *run_target_label_;
    QPushButton *run_button_;
    QPushButton *send_button_;
    QString current_run_target_path_;
    bool current_run_target_can_run_;
};

#endif
