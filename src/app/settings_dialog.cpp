#include "settings_dialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QVariant>
#include <QVBoxLayout>

#include "app_language.h"

SettingsDialog::SettingsDialog(
    ThemeManager::Theme current_theme,
    const CompilerToolchain::Status &toolchain_status,
    QWidget *parent)
    : QDialog(parent),
      theme_combo_(new QComboBox(this)),
      language_combo_(new QComboBox(this)),
      compiler_update_channel_combo_(new QComboBox(this)),
      interpreter_arguments_combo_(new QComboBox(this))
{
    setWindowTitle(tr("Settings"));
    resize(500, 280);

    theme_combo_->addItem(tr("Light"), QVariant::fromValue(0));
    theme_combo_->addItem(tr("Dark"), QVariant::fromValue(1));
    theme_combo_->setCurrentIndex(current_theme == ThemeManager::Theme::Dark ? 1 : 0);

    for (const AppLanguage::Language &language : AppLanguage::available_languages()) {
        language_combo_->addItem(language.name, language.code);
    }

    int selected_language_index =
        language_combo_->findData(AppLanguage::load_language());

    if (selected_language_index >= 0) {
        language_combo_->setCurrentIndex(selected_language_index);
    }

    compiler_update_channel_combo_->addItem(
        tr("Stable releases"),
        QVariant::fromValue(
            static_cast<int>(CompilerToolchain::UpdateChannel::StableRelease)));
    compiler_update_channel_combo_->addItem(
        tr("Develop branch (experimental)"),
        QVariant::fromValue(
            static_cast<int>(CompilerToolchain::UpdateChannel::DevelopBranch)));

    int selected_update_channel_index =
        compiler_update_channel_combo_->findData(
            static_cast<int>(CompilerToolchain::load_update_channel()));

    if (selected_update_channel_index >= 0) {
        compiler_update_channel_combo_->setCurrentIndex(
            selected_update_channel_index);
    }

    interpreter_arguments_combo_->addItem(
        tr("No command-line arguments"),
        QVariant::fromValue(static_cast<int>(InterpreterSettings::ArgumentPreset::None)));
    interpreter_arguments_combo_->addItem(
        "--lang li",
        QVariant::fromValue(
            static_cast<int>(InterpreterSettings::ArgumentPreset::Limburgish)));
    int selected_arguments_index = interpreter_arguments_combo_->findData(
        static_cast<int>(InterpreterSettings::load_argument_preset()));

    if (selected_arguments_index >= 0) {
        interpreter_arguments_combo_->setCurrentIndex(selected_arguments_index);
    }

    auto *form = new QFormLayout();
    form->addRow(tr("Theme"), theme_combo_);
    form->addRow(tr("Language"), language_combo_);
    form->addRow(tr("Compiler update channel"), compiler_update_channel_combo_);
    form->addRow(tr("Interpreter arguments"), interpreter_arguments_combo_);
    form->addRow(tr("Compiler status"), new QLabel(toolchain_status.message, this));
    form->addRow(tr("Toolchain storage"), new QLabel(toolchain_status.storage_root, this));

    if (!toolchain_status.compiler_path.isEmpty()) {
        auto *compiler_path = new QLabel(toolchain_status.compiler_path, this);
        compiler_path->setTextInteractionFlags(Qt::TextSelectableByMouse);
        form->addRow(tr("Compiler path"), compiler_path);
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

InterpreterSettings::ArgumentPreset
SettingsDialog::selected_interpreter_argument_preset() const
{
    return static_cast<InterpreterSettings::ArgumentPreset>(
        interpreter_arguments_combo_->currentData().toInt());
}

CompilerToolchain::UpdateChannel
SettingsDialog::selected_compiler_update_channel() const
{
    return static_cast<CompilerToolchain::UpdateChannel>(
        compiler_update_channel_combo_->currentData().toInt());
}

QString SettingsDialog::selected_language() const
{
    return language_combo_->currentData().toString();
}
