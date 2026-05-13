#ifndef THEME_MANAGER_H
#define THEME_MANAGER_H

#include <QString>

#include <vector>

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
        Dark,
        OneDarkPro,
        Dracula,
        GithubDark,
        Monokai,
        Nord,
        SolarizedDark,
        TokyoNight,
        GruvboxDark,
        CatppuccinMocha,
        AyuDark
    };

    /**
     * Broad theme brightness used for icon and palette choices.
     */
    enum class Appearance {
        Light,
        Dark
    };

    /**
     * Colors used by editor chrome and application controls.
     */
    struct UiColors {
        QString background;
        QString foreground;
        QString border;
        QString selection;
        QString current_line;
        QString cursor;
        QString gutter;
        QString line_number;
        QString line_number_active;
    };

    /**
     * Colors used by source syntax highlighting.
     */
    struct SyntaxColors {
        QString plain;
        QString comment;
        QString keyword;
        QString function;
        QString string;
        QString number;
        QString variable;
        QString property;
        QString class_name;
        QString parameter;
        QString operator_color;
        QString constant;
        QString type;
        QString tag;
        QString attribute;
        QString regexp;
        QString punctuation;
    };

    /**
     * Complete IDE color scheme definition.
     */
    struct ThemeDefinition {
        Theme id;
        QString settings_name;
        QString display_name;
        Appearance appearance;
        UiColors ui;
        SyntaxColors syntax;
    };

    /**
     * Returns every theme exposed by the IDE.
     *
     * @return Theme definitions in menu order.
     */
    static const std::vector<ThemeDefinition> &available_themes();

    /**
     * Returns the definition for a theme.
     *
     * @param theme Theme id.
     * @return Theme definition.
     */
    static const ThemeDefinition &theme_definition(Theme theme);

    /**
     * Applies a theme to the application and stores it in settings.
     *
     * @param app Application instance.
     * @param theme Theme to apply.
     * @param persist Whether to store this theme in settings.
     */
    static void apply_theme(QApplication &app, Theme theme, bool persist = true);

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

    /**
     * Converts a settings string to a theme.
     *
     * @param name Stored theme name.
     * @return Matching theme, or Light when unknown.
     */
    static Theme theme_from_name(const QString &name);

    /**
     * Returns the human-readable theme name.
     *
     * @param theme Theme to convert.
     * @return Display name.
     */
    static QString display_name(Theme theme);

    /**
     * Returns whether a theme is dark.
     *
     * @param theme Theme to inspect.
     * @return True for dark themes.
     */
    static bool is_dark(Theme theme);

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
