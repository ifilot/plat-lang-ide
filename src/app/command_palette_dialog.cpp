#include "command_palette_dialog.h"

#include <QDialogButtonBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QVBoxLayout>

CommandPaletteDialog::CommandPaletteDialog(QWidget *parent)
    : QDialog(parent),
      commands_(new QListWidget(this))
{
    setWindowTitle(tr("Command Palette"));
    resize(420, 420);

    add_command(tr("File: New File"), "new_file");
    add_command(tr("File: Open File"), "open_file");
    add_command(tr("File: Open Folder"), "open_folder");
    add_command(tr("File: Open Example: Hello"), "open_example_hello");
    add_command(tr("File: Open Example: Conditions"), "open_example_conditions");
    add_command(tr("File: Open Example: Fibonacci"), "open_example_fibonacci");
    add_command(tr("File: Open Example: Loops"), "open_example_loops");
    add_command(tr("File: Open Example: Problems"), "open_example_problems");
    add_command(tr("File: Open Example: Tables"), "open_example_tables");
    add_command(tr("File: Save"), "save_file");
    add_command(tr("Run: Run Current File"), "run_current_file");
    add_command(tr("Run: Build"), "build_current_file");
    add_command(tr("Run: Build && Run"), "build_and_run_current_file");
    add_command(tr("View: Toggle File Explorer"), "toggle_file_explorer");
    add_command(tr("View: Toggle Terminal"), "toggle_terminal");
    add_command(tr("View: Toggle Problems"), "toggle_problems");
    add_command(tr("View: Toggle Output"), "toggle_output");
    add_command(tr("View: Toggle Code Assistant"), "toggle_code_assistant");
    add_command(tr("Tools: Format Document"), "format_document");
    add_command(tr("Tools: Settings"), "settings");
    add_command(tr("Help: Documentation"), "documentation");
    add_command(tr("Help: About"), "about");

    commands_->setCurrentRow(0);

    auto *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(commands_, &QListWidget::itemDoubleClicked, this,
            [this](QListWidgetItem *item) {
                Q_UNUSED(item);
                accept();
            });

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->addWidget(commands_, 1);
    layout->addWidget(buttons);
}

QString CommandPaletteDialog::selected_command() const
{
    QListWidgetItem *item = commands_->currentItem();

    if (item == nullptr) {
        return {};
    }

    return item->data(Qt::UserRole).toString();
}

void CommandPaletteDialog::add_command(const QString &title, const QString &id)
{
    auto *item = new QListWidgetItem(title, commands_);
    item->setData(Qt::UserRole, id);
}
