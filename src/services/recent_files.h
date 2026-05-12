#ifndef RECENT_FILES_H
#define RECENT_FILES_H

#include <QString>
#include <QStringList>

/**
 * Stores recently opened editor files.
 */
class RecentFiles {
public:
    /**
     * Returns remembered recent files that still exist.
     *
     * @return Existing recent file paths.
     */
    static QStringList files();

    /**
     * Adds a file path to the top of the recent files list.
     *
     * @param path File path to remember.
     */
    static void remember_file(const QString &path);

    /**
     * Removes a file from the recent files list.
     *
     * @param path File path to remove.
     */
    static void forget_file(const QString &path);
};

#endif
