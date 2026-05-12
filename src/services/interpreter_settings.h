#ifndef INTERPRETER_SETTINGS_H
#define INTERPRETER_SETTINGS_H

#include <QString>
#include <QStringList>

/**
 * Stores interpreter launch preferences.
 */
class InterpreterSettings {
public:
    /**
     * Standard interpreter argument presets.
     */
    enum class ArgumentPreset {
        None,
        Limburgish,
    };

    /**
     * Returns the saved argument preset.
     *
     * @return Current argument preset.
     */
    static ArgumentPreset load_argument_preset();

    /**
     * Stores the selected argument preset.
     *
     * @param preset Argument preset.
     */
    static void save_argument_preset(ArgumentPreset preset);

    /**
     * Returns command-line arguments for a preset.
     *
     * @param preset Argument preset.
     * @return Arguments to pass before the source file.
     */
    static QStringList arguments_for_preset(ArgumentPreset preset);

    /**
     * Returns a stable settings value for a preset.
     *
     * @param preset Argument preset.
     * @return Settings value.
     */
    static QString preset_name(ArgumentPreset preset);
};

#endif
