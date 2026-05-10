#include "command_palette_dialog.h"

#include <QDialogButtonBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QVBoxLayout>

CommandPaletteDialog::CommandPaletteDialog(QWidget *parent)
    : QDialog(parent),
      commands_(new QListWidget(this))
{
    setWindowTitle("Command Palette");
    resize(420, 420);

    add_command("File: New File", "new_file");
    add_command("File: Open File", "open_file");
    add_command("File: Open Folder", "open_folder");
    add_command("File: Save", "save_file");
    add_command("Run: Run Current File", "run_current_file");
    add_command("Run: Build", "build_current_file");
    add_command("Run: Build && Run", "build_and_run_current_file");
    add_command("View: Toggle File Explorer", "toggle_file_explorer");
    add_command("View: Toggle Terminal", "toggle_terminal");
    add_command("View: Toggle Problems", "toggle_problems");
    add_command("View: Toggle Output", "toggle_output");
    add_command("View: Toggle Code Assistant", "toggle_code_assistant");
    add_command("Tools: Format Document", "format_document");
    add_command("Tools: Settings", "settings");
    add_command("Help: Documentation", "documentation");
    add_command("Help: About", "about");

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
