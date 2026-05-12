#ifndef APP_LANGUAGE_H
#define APP_LANGUAGE_H

#include <QString>
#include <QVector>

class QCoreApplication;

/**
 * Stores and applies the user's UI language preference.
 */
class AppLanguage {
public:
    struct Language {
        QString code;
        QString name;
    };

    /**
     * Returns languages exposed in the settings dialog.
     *
     * @return Available UI languages.
     */
    static QVector<Language> available_languages();

    /**
     * Loads the saved language code.
     *
     * @return Saved language code, or system for the operating-system locale.
     */
    static QString load_language();

    /**
     * Saves the language code.
     *
     * @param language_code Language code from available_languages.
     */
    static void save_language(const QString &language_code);

    /**
     * Installs the matching Qt translator.
     *
     * @param app Application instance.
     * @return True when a translation file was loaded.
     */
    static bool install_translator(QCoreApplication &app);

private:
    static QString resolved_locale_name(const QString &language_code);
};

#endif
