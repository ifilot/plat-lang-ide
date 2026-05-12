#include "interpreter_settings.h"

#include <QSettings>

namespace {
constexpr const char *argument_preset_key = "interpreter/argumentPreset";
constexpr const char *none_preset_name = "none";
constexpr const char *limburgish_preset_name = "limburgish";
}

InterpreterSettings::ArgumentPreset InterpreterSettings::load_argument_preset()
{
    QSettings settings;
    QString name = settings.value(argument_preset_key, none_preset_name).toString();

    if (name == limburgish_preset_name) {
        return ArgumentPreset::Limburgish;
    }

    return ArgumentPreset::None;
}

void InterpreterSettings::save_argument_preset(ArgumentPreset preset)
{
    QSettings settings;
    settings.setValue(argument_preset_key, preset_name(preset));
}

QStringList InterpreterSettings::arguments_for_preset(ArgumentPreset preset)
{
    switch (preset) {
    case ArgumentPreset::Limburgish:
        return {"--lang", "li"};
    case ArgumentPreset::None:
        return {};
    }

    return {};
}

QString InterpreterSettings::preset_name(ArgumentPreset preset)
{
    switch (preset) {
    case ArgumentPreset::Limburgish:
        return limburgish_preset_name;
    case ArgumentPreset::None:
        return none_preset_name;
    }

    return none_preset_name;
}
