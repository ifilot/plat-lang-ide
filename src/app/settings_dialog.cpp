#include "settings_dialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QVariant>
#include <QVBoxLayout>

#include "app_language.h"
#include "code_editor.h"

SettingsDialog::SettingsDialog(
    ThemeManager::Theme current_theme,
    const CompilerToolchain::Status &toolchain_status,
    QWidget *parent)
    : QDialog(parent),
      theme_combo_(new QComboBox(this)),
      theme_preview_editor_(new CodeEditor(this)),
      language_combo_(new QComboBox(this)),
      compiler_update_channel_combo_(new QComboBox(this)),
      interpreter_arguments_combo_(new QComboBox(this))
{
    setWindowTitle(tr("Settings"));
    resize(640, 520);

    for (const ThemeManager::ThemeDefinition &theme :
         ThemeManager::available_themes()) {
        theme_combo_->addItem(theme.display_name,
                              QVariant::fromValue(static_cast<int>(theme.id)));
    }

    int selected_theme_index =
        theme_combo_->findData(static_cast<int>(current_theme));

    if (selected_theme_index >= 0) {
        theme_combo_->setCurrentIndex(selected_theme_index);
    }

    theme_preview_editor_->setReadOnly(true);
    theme_preview_editor_->setFixedHeight(170);
    theme_preview_editor_->setPlainText(
        QStringLiteral("# Theme preview\n"
                       "funksie fib(nommer n) {\n"
                       "    angesj n <= 1 {\n"
                       "        trok n\n"
                       "    }\n"
                       "\n"
                       "    trok fib(n - 1) + fib(n - 2)\n"
                       "}\n"
                       "\n"
                       "aafdrokke(\"fib = \" + fib(6))\n"));
    theme_preview_editor_->apply_theme(current_theme);

    connect(theme_combo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int) {
                ThemeManager::Theme theme = selected_theme();
                theme_preview_editor_->apply_theme(theme);
                emit theme_preview_requested(theme);
            });

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
    form->addRow(tr("Preview"), theme_preview_editor_);
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
    return static_cast<ThemeManager::Theme>(theme_combo_->currentData().toInt());
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
