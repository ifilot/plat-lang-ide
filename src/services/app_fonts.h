#ifndef APP_FONTS_H
#define APP_FONTS_H

#include <QFont>

/**
 * Provides bundled application fonts.
 */
class AppFonts {
public:
    /**
     * Returns the bundled Space Mono font configured for code-like views.
     *
     * @param point_size Font point size.
     * @return Monospace application font.
     */
    static QFont code_font(int point_size);

private:
    /**
     * Loads Space Mono and returns its family name.
     *
     * @return Space Mono family name, or monospace fallback.
     */
    static QString space_mono_family();
};

#endif
