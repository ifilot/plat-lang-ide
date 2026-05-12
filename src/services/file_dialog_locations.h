#ifndef FILE_DIALOG_LOCATIONS_H
#define FILE_DIALOG_LOCATIONS_H

#include <QString>

/**
 * Stores and resolves the folder used when opening file dialogs.
 */
class FileDialogLocations {
public:
    /**
     * Returns the remembered dialog folder, or the user's home folder.
     *
     * @return Existing folder path.
     */
    static QString dialog_directory();

    /**
     * Builds a dialog start path in the remembered folder.
     *
     * @param file_name Optional file name to append.
     * @return Folder path, or folder/file_name when a file name is provided.
     */
    static QString start_path(const QString &file_name = {});

    /**
     * Stores a folder as the next file dialog location.
     *
     * @param path Folder path.
     */
    static void remember_directory(const QString &path);

    /**
     * Stores a file's parent folder as the next file dialog location.
     *
     * @param path File path.
     */
    static void remember_file_path(const QString &path);
};

#endif
