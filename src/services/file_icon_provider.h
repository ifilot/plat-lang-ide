#ifndef FILE_ICON_PROVIDER_H
#define FILE_ICON_PROVIDER_H

#include <QFileIconProvider>
#include <QIcon>

/**
 * Provides compact IDE-style icons for files and folders.
 */
class FileIconProvider : public QFileIconProvider {
public:
    /**
     * Returns an icon for a file-system entry.
     *
     * @param info File information.
     * @return Icon for the entry.
     */
    QIcon icon(const QFileInfo &info) const override;

private:
    /**
     * Returns the bundled Font Awesome folder icon.
     *
     * @return Folder icon.
     */
    QIcon folder_icon() const;

    /**
     * Returns the bundled Font Awesome file icon.
     *
     * @return File icon.
     */
    QIcon file_icon() const;
};

#endif
