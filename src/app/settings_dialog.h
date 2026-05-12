#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

#include <QDialog>

#include "compiler_toolchain.h"
#include "interpreter_settings.h"
#include "theme_manager.h"

class QComboBox;

/**
 * Small settings dialog for application-level preferences.
 */
class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    /**
     * Creates the settings dialog.
     *
     * @param current_theme Active theme.
     * @param toolchain_status Current compiler status.
     * @param parent Optional parent widget.
     */
    explicit SettingsDialog(ThemeManager::Theme current_theme,
                            const CompilerToolchain::Status &toolchain_status,
                            QWidget *parent = nullptr);

    /**
     * Returns the selected theme.
     *
     * @return Selected theme.
     */
    ThemeManager::Theme selected_theme() const;

    /**
     * Returns the selected interpreter argument preset.
     *
     * @return Selected interpreter argument preset.
     */
    InterpreterSettings::ArgumentPreset selected_interpreter_argument_preset() const;

    /**
     * Returns the selected compiler update channel.
     *
     * @return Selected compiler update channel.
     */
    CompilerToolchain::UpdateChannel selected_compiler_update_channel() const;

    /**
     * Returns the selected UI language code.
     *
     * @return Selected language code.
     */
    QString selected_language() const;

private:
    QComboBox *theme_combo_;
    QComboBox *language_combo_;
    QComboBox *compiler_update_channel_combo_;
    QComboBox *interpreter_arguments_combo_;
};

#endif
