#include "settings_dialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QVariant>
#include <QVBoxLayout>

SettingsDialog::SettingsDialog(
    ThemeManager::Theme current_theme,
    const CompilerToolchain::Status &toolchain_status,
    QWidget *parent)
    : QDialog(parent),
      theme_combo_(new QComboBox(this))
{
    setWindowTitle("Settings");
    resize(460, 240);

    theme_combo_->addItem("Light", QVariant::fromValue(0));
    theme_combo_->addItem("Dark", QVariant::fromValue(1));
    theme_combo_->setCurrentIndex(current_theme == ThemeManager::Theme::Dark ? 1 : 0);

    auto *form = new QFormLayout();
    form->addRow("Theme", theme_combo_);
    form->addRow("Compiler status", new QLabel(toolchain_status.message, this));
    form->addRow("Toolchain storage", new QLabel(toolchain_status.storage_root, this));

    if (!toolchain_status.compiler_path.isEmpty()) {
        auto *compiler_path = new QLabel(toolchain_status.compiler_path, this);
        compiler_path->setTextInteractionFlags(Qt::TextSelectableByMouse);
        form->addRow("Compiler path", compiler_path);
    }

    auto *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(14, 14, 14, 14);
    layout->addLayout(form);
    layout->addStretch(1);
    layout->addWidget(buttons);
}

ThemeManager::Theme SettingsDialog::selected_theme() const
{
    return theme_combo_->currentIndex() == 1
               ? ThemeManager::Theme::Dark
               : ThemeManager::Theme::Light;
}
