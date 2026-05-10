#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

#include <QDialog>

#include "compiler_toolchain.h"
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

private:
    QComboBox *theme_combo_;
};

#endif
