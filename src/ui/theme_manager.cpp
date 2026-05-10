#include "theme_manager.h"

#include <QApplication>
#include <QFile>
#include <QPalette>
#include <QSettings>
#include <QStyle>

namespace {
constexpr const char *theme_setting_key = "appearance/theme";

QString load_style_sheet(ThemeManager::Theme theme)
{
    QString path = theme == ThemeManager::Theme::Dark
                       ? ":/assets/themes/dark.qss"
                       : ":/assets/themes/light.qss";
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    return QString::fromUtf8(file.readAll());
}
}

void ThemeManager::apply_theme(QApplication &app, Theme theme)
{
    QPalette palette;

    if (theme == Theme::Dark) {
        palette.setColor(QPalette::Window, QColor(31, 34, 39));
        palette.setColor(QPalette::WindowText, QColor(231, 234, 238));
        palette.setColor(QPalette::Base, QColor(24, 26, 30));
        palette.setColor(QPalette::AlternateBase, QColor(37, 41, 47));
        palette.setColor(QPalette::ToolTipBase, QColor(231, 234, 238));
        palette.setColor(QPalette::ToolTipText, QColor(24, 26, 30));
        palette.setColor(QPalette::Text, QColor(231, 234, 238));
        palette.setColor(QPalette::Button, QColor(43, 48, 55));
        palette.setColor(QPalette::ButtonText, QColor(231, 234, 238));
        palette.setColor(QPalette::BrightText, QColor(255, 255, 255));
        palette.setColor(QPalette::Highlight, QColor(72, 118, 214));
        palette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
        palette.setColor(QPalette::PlaceholderText, QColor(145, 152, 164));
    } else {
        palette = QApplication::style()->standardPalette();
        palette.setColor(QPalette::Window, QColor(246, 247, 249));
        palette.setColor(QPalette::Base, QColor(255, 255, 255));
        palette.setColor(QPalette::AlternateBase, QColor(240, 242, 245));
        palette.setColor(QPalette::Highlight, QColor(42, 105, 210));
        palette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
    }

    app.setPalette(palette);
    app.setStyleSheet(style_sheet(theme));

    QSettings settings;
    settings.setValue(theme_setting_key, theme_name(theme));
}

ThemeManager::Theme ThemeManager::load_theme()
{
    QSettings settings;
    QString name = settings.value(theme_setting_key, "light").toString();

    if (name == "dark") {
        return Theme::Dark;
    }

    return Theme::Light;
}

QString ThemeManager::theme_name(Theme theme)
{
    return theme == Theme::Dark ? "dark" : "light";
}

QString ThemeManager::style_sheet(Theme theme)
{
    return load_style_sheet(theme);
}

