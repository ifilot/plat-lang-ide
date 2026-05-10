#include "app_fonts.h"

#include <QFontDatabase>
#include <QStringList>

QFont AppFonts::code_font(int point_size)
{
    QFont font(space_mono_family(), point_size);
    font.setFixedPitch(true);
    font.setStyleHint(QFont::Monospace);
    return font;
}

QString AppFonts::space_mono_family()
{
    static QString family;
    static bool initialized = false;

    if (initialized) {
        return family;
    }

    initialized = true;
    const int regular_id = QFontDatabase::addApplicationFont(
        ":/assets/fonts/space-mono/SpaceMono-Regular.ttf");
    QFontDatabase::addApplicationFont(
        ":/assets/fonts/space-mono/SpaceMono-Bold.ttf");

    if (regular_id >= 0) {
        const QStringList families =
            QFontDatabase::applicationFontFamilies(regular_id);

        if (!families.isEmpty()) {
            family = families.first();
        }
    }

    if (family.isEmpty()) {
        family = "monospace";
    }

    return family;
}
