#include "theme_manager.h"

#include <QApplication>
#include <QColor>
#include <QPalette>
#include <QSettings>
#include <QStyle>

#include <algorithm>

namespace {
constexpr const char *theme_setting_key = "appearance/theme";

QColor color(const QString &hex)
{
    return QColor(hex);
}

QString color_name(const QColor &color)
{
    return color.name(QColor::HexRgb);
}

QString mix(const QString &first, const QString &second, double second_weight)
{
    QColor a(first);
    QColor b(second);
    const double first_weight = 1.0 - second_weight;

    return color_name(QColor(qRound(a.red() * first_weight + b.red() * second_weight),
                             qRound(a.green() * first_weight + b.green() * second_weight),
                             qRound(a.blue() * first_weight + b.blue() * second_weight)));
}

QString hover_color(const ThemeManager::ThemeDefinition &theme)
{
    return mix(theme.ui.current_line, theme.ui.foreground,
               theme.appearance == ThemeManager::Appearance::Dark ? 0.10 : 0.06);
}

QString button_color(const ThemeManager::ThemeDefinition &theme)
{
    return theme.appearance == ThemeManager::Appearance::Dark
               ? mix(theme.ui.background, theme.ui.border, 0.55)
               : "#ffffff";
}

QString disabled_color(const ThemeManager::ThemeDefinition &theme)
{
    return mix(theme.ui.foreground, theme.ui.background, 0.48);
}

const ThemeManager::ThemeDefinition kFallbackLightTheme = {
    ThemeManager::Theme::Light,
    "light",
    "Light",
    ThemeManager::Appearance::Light,
    {
        "#f6f7f9", "#20242a", "#d7dbe2", "#2a69d2", "#eef1f5",
        "#2a69d2", "#eef1f5", "#8b93a0", "#20242a"
    },
    {
        "#20242a", "#64737d", "#205295", "#76489e", "#924822",
        "#54782a", "#20242a", "#76625e", "#895900", "#895900",
        "#505660", "#6c4aa0", "#895900", "#205295", "#76489e",
        "#924822", "#20242a"
    }
};
}

const std::vector<ThemeManager::ThemeDefinition> &ThemeManager::available_themes()
{
    static const std::vector<ThemeDefinition> themes = {
        kFallbackLightTheme,
        {
            Theme::Dark,
            "dark",
            "Dark",
            Appearance::Dark,
            {
                "#1f2227", "#e7eaee", "#343a44", "#4876d6", "#252930",
                "#7aa2f7", "#252930", "#9198a4", "#ffffff"
            },
            {
                "#e7eaee", "#565f89", "#7aa2f7", "#7dcfff", "#9ece6a",
                "#ff9e64", "#e7eaee", "#7dcfff", "#e0af68", "#e0af68",
                "#c0caf5", "#bb9af7", "#e0af68", "#7aa2f7", "#7dcfff",
                "#9ece6a", "#c0caf5"
            }
        },
        {
            Theme::OneDarkPro,
            "one-dark-pro",
            "One Dark Pro",
            Appearance::Dark,
            {
                "#282c34", "#abb2bf", "#3e4451", "#3e4451", "#2c313c",
                "#528bff", "#282c34", "#5c6370", "#abb2bf"
            },
            {
                "#abb2bf", "#5c6370", "#c678dd", "#61afef", "#98c379",
                "#d19a66", "#e06c75", "#e5c07b", "#e5c07b", "#d19a66",
                "#56b6c2", "#d19a66", "#e5c07b", "#e06c75", "#d19a66",
                "#56b6c2", "#abb2bf"
            }
        },
        {
            Theme::Dracula,
            "dracula",
            "Dracula",
            Appearance::Dark,
            {
                "#282a36", "#f8f8f2", "#44475a", "#44475a", "#44475a",
                "#f8f8f2", "#282a36", "#6272a4", "#f8f8f2"
            },
            {
                "#f8f8f2", "#6272a4", "#ff79c6", "#50fa7b", "#f1fa8c",
                "#bd93f9", "#8be9fd", "#ffb86c", "#8be9fd", "#ffb86c",
                "#ff79c6", "#bd93f9", "#8be9fd", "#ff79c6", "#50fa7b",
                "#f1fa8c", "#f8f8f2"
            }
        },
        {
            Theme::GithubDark,
            "github-dark",
            "GitHub Dark",
            Appearance::Dark,
            {
                "#0d1117", "#c9d1d9", "#30363d", "#264f78", "#161b22",
                "#58a6ff", "#0d1117", "#6e7681", "#c9d1d9"
            },
            {
                "#c9d1d9", "#8b949e", "#ff7b72", "#d2a8ff", "#a5d6ff",
                "#79c0ff", "#ffa657", "#79c0ff", "#ffa657", "#ffa657",
                "#ff7b72", "#79c0ff", "#ffa657", "#7ee787", "#79c0ff",
                "#a5d6ff", "#c9d1d9"
            }
        },
        {
            Theme::Monokai,
            "monokai",
            "Monokai",
            Appearance::Dark,
            {
                "#272822", "#f8f8f2", "#3e3d32", "#49483e", "#3e3d32",
                "#f8f8f0", "#272822", "#90908a", "#f8f8f2"
            },
            {
                "#f8f8f2", "#75715e", "#f92672", "#a6e22e", "#e6db74",
                "#ae81ff", "#fd971f", "#66d9ef", "#a6e22e", "#fd971f",
                "#f92672", "#ae81ff", "#66d9ef", "#f92672", "#a6e22e",
                "#e6db74", "#f8f8f2"
            }
        },
        {
            Theme::Nord,
            "nord",
            "Nord",
            Appearance::Dark,
            {
                "#2e3440", "#d8dee9", "#4c566a", "#434c5e", "#3b4252",
                "#d8dee9", "#2e3440", "#616e88", "#eceff4"
            },
            {
                "#d8dee9", "#616e88", "#81a1c1", "#88c0d0", "#a3be8c",
                "#b48ead", "#d8dee9", "#8fbcbb", "#ebcb8b", "#d08770",
                "#81a1c1", "#b48ead", "#8fbcbb", "#81a1c1", "#8fbcbb",
                "#ebcb8b", "#d8dee9"
            }
        },
        {
            Theme::SolarizedDark,
            "solarized-dark",
            "Solarized Dark",
            Appearance::Dark,
            {
                "#002b36", "#839496", "#073642", "#073642", "#073642",
                "#93a1a1", "#002b36", "#586e75", "#93a1a1"
            },
            {
                "#839496", "#586e75", "#859900", "#268bd2", "#2aa198",
                "#d33682", "#b58900", "#cb4b16", "#b58900", "#93a1a1",
                "#6c71c4", "#d33682", "#b58900", "#268bd2", "#2aa198",
                "#cb4b16", "#839496"
            }
        },
        {
            Theme::TokyoNight,
            "tokyo-night",
            "Tokyo Night",
            Appearance::Dark,
            {
                "#1a1b26", "#c0caf5", "#292e42", "#33467c", "#24283b",
                "#c0caf5", "#1a1b26", "#565f89", "#c0caf5"
            },
            {
                "#c0caf5", "#565f89", "#bb9af7", "#7aa2f7", "#9ece6a",
                "#ff9e64", "#c0caf5", "#73daca", "#e0af68", "#e0af68",
                "#89ddff", "#ff9e64", "#2ac3de", "#f7768e", "#e0af68",
                "#b4f9f8", "#c0caf5"
            }
        },
        {
            Theme::GruvboxDark,
            "gruvbox-dark",
            "Gruvbox Dark",
            Appearance::Dark,
            {
                "#282828", "#ebdbb2", "#504945", "#504945", "#3c3836",
                "#ebdbb2", "#282828", "#928374", "#ebdbb2"
            },
            {
                "#ebdbb2", "#928374", "#fb4934", "#b8bb26", "#b8bb26",
                "#d3869b", "#fabd2f", "#83a598", "#fabd2f", "#fe8019",
                "#fe8019", "#d3869b", "#8ec07c", "#fb4934", "#fabd2f",
                "#b8bb26", "#ebdbb2"
            }
        },
        {
            Theme::CatppuccinMocha,
            "catppuccin-mocha",
            "Catppuccin Mocha",
            Appearance::Dark,
            {
                "#1e1e2e", "#cdd6f4", "#45475a", "#45475a", "#313244",
                "#f5e0dc", "#1e1e2e", "#6c7086", "#cdd6f4"
            },
            {
                "#cdd6f4", "#6c7086", "#cba6f7", "#89b4fa", "#a6e3a1",
                "#fab387", "#f38ba8", "#94e2d5", "#f9e2af", "#eba0ac",
                "#89dceb", "#fab387", "#f9e2af", "#f38ba8", "#fab387",
                "#94e2d5", "#cdd6f4"
            }
        },
        {
            Theme::AyuDark,
            "ayu-dark",
            "Ayu Dark",
            Appearance::Dark,
            {
                "#0f1419", "#b3b1ad", "#253340", "#253340", "#131721",
                "#f29718", "#0f1419", "#5c6773", "#b3b1ad"
            },
            {
                "#b3b1ad", "#5c6773", "#ff8f40", "#ffb454", "#aad94c",
                "#d2a6ff", "#f07178", "#59c2ff", "#73d0ff", "#e6b673",
                "#f29668", "#d2a6ff", "#73d0ff", "#ff8f40", "#ffb454",
                "#95e6cb", "#b3b1ad"
            }
        }
    };

    return themes;
}

const ThemeManager::ThemeDefinition &ThemeManager::theme_definition(Theme theme)
{
    const auto &themes = available_themes();
    auto match = std::find_if(themes.begin(), themes.end(),
                              [theme](const ThemeDefinition &definition) {
                                  return definition.id == theme;
                              });

    return match != themes.end() ? *match : kFallbackLightTheme;
}

void ThemeManager::apply_theme(QApplication &app, Theme theme, bool persist)
{
    const ThemeDefinition &definition = theme_definition(theme);
    QPalette palette = QApplication::style()->standardPalette();

    palette.setColor(QPalette::Window, color(definition.ui.background));
    palette.setColor(QPalette::WindowText, color(definition.ui.foreground));
    palette.setColor(QPalette::Base, color(definition.ui.background));
    palette.setColor(QPalette::AlternateBase, color(definition.ui.gutter));
    palette.setColor(QPalette::ToolTipBase, color(definition.ui.foreground));
    palette.setColor(QPalette::ToolTipText, color(definition.ui.background));
    palette.setColor(QPalette::Text, color(definition.ui.foreground));
    palette.setColor(QPalette::Button, color(button_color(definition)));
    palette.setColor(QPalette::ButtonText, color(definition.ui.foreground));
    palette.setColor(QPalette::BrightText, QColor(255, 255, 255));
    palette.setColor(QPalette::Highlight, color(definition.ui.selection));
    palette.setColor(QPalette::HighlightedText, color(definition.ui.foreground));
    palette.setColor(QPalette::PlaceholderText, color(definition.ui.line_number));

    app.setPalette(palette);
    app.setStyleSheet(style_sheet(theme));

    if (persist) {
        QSettings settings;
        settings.setValue(theme_setting_key, definition.settings_name);
    }
}

ThemeManager::Theme ThemeManager::load_theme()
{
    QSettings settings;
    return theme_from_name(settings.value(theme_setting_key, "light").toString());
}

QString ThemeManager::theme_name(Theme theme)
{
    return theme_definition(theme).settings_name;
}

ThemeManager::Theme ThemeManager::theme_from_name(const QString &name)
{
    const auto &themes = available_themes();
    auto match = std::find_if(themes.begin(), themes.end(),
                              [&name](const ThemeDefinition &definition) {
                                  return definition.settings_name == name;
                              });

    return match != themes.end() ? match->id : Theme::Light;
}

QString ThemeManager::display_name(Theme theme)
{
    return theme_definition(theme).display_name;
}

bool ThemeManager::is_dark(Theme theme)
{
    return theme_definition(theme).appearance == Appearance::Dark;
}

QString ThemeManager::style_sheet(Theme theme)
{
    const ThemeDefinition &definition = theme_definition(theme);
    const QString background = definition.ui.background;
    const QString foreground = definition.ui.foreground;
    const QString border = definition.ui.border;
    const QString selection = definition.ui.selection;
    const QString current_line = definition.ui.current_line;
    const QString gutter = definition.ui.gutter;
    const QString line_number = definition.ui.line_number;
    const QString active = definition.ui.line_number_active;
    const QString button = button_color(definition);
    const QString button_hover = hover_color(definition);
    const QString button_pressed = mix(selection, definition.ui.cursor, 0.25);
    const QString muted = disabled_color(definition);
    const QString scroll = mix(border, foreground, 0.18);
    const QString scroll_hover = mix(border, foreground, 0.28);
    const QString scroll_pressed = mix(border, foreground, 0.38);

    QString style = QString(R"(
QMainWindow, QWidget {
    background: %1;
    color: %2;
}

QMenuBar, QMenu {
    background: %6;
    color: %2;
    border: 1px solid %3;
}

QMenu {
    padding: 4px 0;
}

QMenu::item {
    min-height: 20px;
    padding: 4px 18px 4px 22px;
}

QMenuBar::item:selected, QMenu::item:selected {
    background: %5;
}

#titleBarMenu {
    background: transparent;
    border: 0;
}

#titleBarMenu::item {
    background: transparent;
    padding: 6px 8px;
    margin: 0;
}

#titleBarMenu::item:selected {
    background: %5;
    border-radius: 3px;
}

#customTitleBar {
    background: %6;
    border-bottom: 1px solid %3;
}

#titleBarTitle {
    color: %2;
    font-weight: 600;
    padding-right: 8px;
}

#customTitleBar QPushButton {
    background: transparent;
    color: %11;
    border: 0;
    border-radius: 0;
    padding: 0;
}

#customTitleBar QPushButton:hover {
    background: %5;
    color: %12;
}

#customTitleBar #titleBarCloseButton:hover {
    background: #d83b3b;
    color: #ffffff;
}

#activityBar {
    background: %6;
    border-right: 1px solid %3;
}

#activityBar QPushButton {
    background: transparent;
    color: %11;
    border: 0;
    border-radius: 5px;
    font-size: 15px;
    padding: 0;
}

#activityBar QPushButton:hover {
    background: %5;
    color: %12;
}

#activityBar QPushButton:pressed {
    background: %4;
    color: %12;
}

QTreeView, QTextEdit, QTextBrowser, QPlainTextEdit, QLineEdit, QComboBox {
    background: %1;
    color: %2;
    border: 1px solid %3;
    selection-background-color: %4;
    selection-color: %2;
}

QComboBox {
    padding: 3px 7px;
}

QComboBox QAbstractItemView {
    background: %1;
    color: %2;
    border: 1px solid %3;
    selection-background-color: %4;
}

#findReplaceBar {
    background: %1;
    border-bottom: 1px solid %3;
}

#findReplaceBar QLabel {
    color: %11;
}

#findReplaceBar QLineEdit {
    min-height: 24px;
    border-radius: 4px;
    padding: 2px 7px;
}

#findBarButton {
    min-height: 24px;
    padding: 2px 9px;
}

#findBarCloseButton {
    background: transparent;
    border: 0;
    border-radius: 4px;
    padding: 0;
}

#findBarCloseButton:hover {
    background: %5;
}

QHeaderView::section {
    background: %6;
    color: %2;
    border: 0;
    border-bottom: 1px solid %3;
    padding: 4px;
}

QTabWidget::pane {
    border: 0;
    border-top: 1px solid %3;
    background: %1;
}

QTabWidget::tab-bar {
    left: 0;
    top: 0;
}

QTabBar {
    background: %1;
    border: 0;
}

QTabBar::tab {
    background: %6;
    color: %11;
    border: 1px solid %3;
    border-bottom: 0;
    padding: 6px 12px;
}

QTabBar::tab:selected {
    background: %1;
    color: %12;
    border-bottom: 1px solid %3;
}

#tabCloseButton {
    background: transparent;
    border: 0;
    border-radius: 4px;
    padding: 0;
}

#tabCloseButton:hover {
    background: %5;
}

#tabCloseButton:pressed {
    background: %10;
}

QPushButton {
    background: %7;
    color: %2;
    border: 1px solid %3;
    border-radius: 4px;
    padding: 5px 12px;
}

QPushButton:hover {
    background: %8;
}

QPushButton:pressed {
    background: %10;
}

QPushButton:disabled {
    color: %11;
    background: %6;
}

QSplitter::handle {
    background: transparent;
}

QSplitter::handle:horizontal {
    width: 7px;
    image: none;
    border-left: 3px solid transparent;
    border-right: 3px solid transparent;
    background-color: %3;
    background-clip: content;
}

QSplitter::handle:vertical {
    height: 7px;
    image: none;
    border-top: 3px solid transparent;
    border-bottom: 3px solid transparent;
    background-color: %3;
    background-clip: content;
}

QSplitter::handle:hover {
    background-color: %4;
}

QSplitter::handle:pressed {
    background-color: %10;
}

QScrollBar:vertical {
    background: transparent;
    border: 0;
    width: 10px;
    margin: 0;
}

QScrollBar::handle:vertical {
    background: %13;
    border-radius: 5px;
    min-height: 28px;
    margin: 2px;
}

QScrollBar::handle:vertical:hover {
    background: %14;
}

QScrollBar::handle:vertical:pressed {
    background: %15;
}

QScrollBar::add-line:vertical,
QScrollBar::sub-line:vertical,
QScrollBar::add-page:vertical,
QScrollBar::sub-page:vertical {
    background: transparent;
    border: 0;
    height: 0;
}

QScrollBar:horizontal {
    background: transparent;
    border: 0;
    height: 10px;
    margin: 0;
}

QScrollBar::handle:horizontal {
    background: %13;
    border-radius: 5px;
    min-width: 28px;
    margin: 2px;
}

QScrollBar::handle:horizontal:hover {
    background: %14;
}

QScrollBar::handle:horizontal:pressed {
    background: %15;
}

QScrollBar::add-line:horizontal,
QScrollBar::sub-line:horizontal,
QScrollBar::add-page:horizontal,
QScrollBar::sub-page:horizontal {
    background: transparent;
    border: 0;
    width: 0;
}

QLabel {
    color: %11;
}
)");

    style.replace("%15", scroll_pressed);
    style.replace("%14", scroll_hover);
    style.replace("%13", scroll);
    style.replace("%12", active);
    style.replace("%11", muted);
    style.replace("%10", button_pressed);
    style.replace("%9", line_number);
    style.replace("%8", button_hover);
    style.replace("%7", button);
    style.replace("%6", gutter);
    style.replace("%5", current_line);
    style.replace("%4", selection);
    style.replace("%3", border);
    style.replace("%2", foreground);
    style.replace("%1", background);

    return style;
}
