#include "app_language.h"

#include <QCoreApplication>
#include <QLocale>
#include <QSettings>
#include <QTranslator>

namespace {
constexpr const char *language_setting_key = "appearance/language";
constexpr const char *system_language_code = "system";
}

QVector<AppLanguage::Language> AppLanguage::available_languages()
{
    return {
        {system_language_code,
         QCoreApplication::translate("AppLanguage", "System default")},
        {"en",
         QCoreApplication::translate("AppLanguage", "English")},
        {"nl",
         QCoreApplication::translate("AppLanguage", "Dutch")},
        {"li",
         QCoreApplication::translate("AppLanguage", "Limburgish")},
    };
}

QString AppLanguage::load_language()
{
    QSettings settings;
    return settings.value(language_setting_key, system_language_code).toString();
}

void AppLanguage::save_language(const QString &language_code)
{
    QSettings settings;
    settings.setValue(language_setting_key, language_code);
}

bool AppLanguage::install_translator(QCoreApplication &app)
{
    static QTranslator translator;

    app.removeTranslator(&translator);

    const QString locale_name = resolved_locale_name(load_language());

    if (locale_name.startsWith("en")) {
        return false;
    }

    const QString full_file = "plat_lang_ide_" + locale_name;

    if (translator.load(full_file, ":/i18n")) {
        app.installTranslator(&translator);
        return true;
    }

    const QString language = locale_name.section('_', 0, 0);

    if (!language.isEmpty()
        && translator.load("plat_lang_ide_" + language, ":/i18n")) {
        app.installTranslator(&translator);
        return true;
    }

    return false;
}

QString AppLanguage::resolved_locale_name(const QString &language_code)
{
    if (language_code == system_language_code || language_code.isEmpty()) {
        return QLocale::system().name();
    }

    return language_code;
}
