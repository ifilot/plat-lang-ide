#include "file_icon_provider.h"

#include <QFileInfo>
#include <QPainter>
#include <QPixmap>
#include <QSvgRenderer>

namespace {
/**
 * Renders a bundled Font Awesome SVG resource into a navigator icon.
 *
 * @param resource_path Qt resource path to the SVG.
 * @return Rendered icon.
 */
QIcon render_svg_icon(const QString &resource_path)
{
    QPixmap pixmap(16, 16);
    pixmap.fill(Qt::transparent);

    QSvgRenderer renderer(resource_path);

    if (!renderer.isValid()) {
        return QIcon();
    }

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    renderer.render(&painter, QRectF(1.0, 1.0, 14.0, 14.0));

    return QIcon(pixmap);
}
}

QIcon FileIconProvider::icon(const QFileInfo &info) const
{
    if (info.isDir()) {
        return folder_icon();
    }

    return file_icon();
}

QIcon FileIconProvider::folder_icon() const
{
    static const QIcon icon =
        render_svg_icon(":/assets/icons/fontawesome/solid/folder.svg");
    return icon;
}

QIcon FileIconProvider::file_icon() const
{
    static const QIcon icon =
        render_svg_icon(":/assets/icons/fontawesome/solid/file.svg");
    return icon;
}
