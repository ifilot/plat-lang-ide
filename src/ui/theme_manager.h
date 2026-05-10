#ifndef THEME_MANAGER_H
#define THEME_MANAGER_H

#include <QString>

class QApplication;

/**
 * Applies and persists the IDE visual theme.
 */
class ThemeManager {
public:
    /**
     * Available application themes.
     */
    enum class Theme {
        Light,
        Dark
    };

    /**
     * Applies a theme to the application and stores it in settings.
     *
     * @param app Application instance.
     * @param theme Theme to apply.
     */
    static void apply_theme(QApplication &app, Theme theme);

    /**
     * Loads the persisted theme from settings.
     *
     * @return Persisted theme, or the default light theme.
     */
    static Theme load_theme();

    /**
     * Converts a theme to a settings string.
     *
     * @param theme Theme to convert.
     * @return Theme name.
     */
    static QString theme_name(Theme theme);

private:
    /**
     * Builds the application stylesheet for a theme.
     *
     * @param theme Theme to style.
     * @return Qt stylesheet.
     */
    static QString style_sheet(Theme theme);
};

#endif
