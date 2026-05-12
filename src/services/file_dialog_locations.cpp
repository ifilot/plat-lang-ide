#include "file_dialog_locations.h"

#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>

namespace {
constexpr const char *last_dialog_directory_key = "fileDialogs/lastDirectory";

QString home_directory()
{
    QString home = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);

    if (home.isEmpty()) {
        home = QDir::homePath();
    }

    QFileInfo home_info(home);

    if (home_info.isDir()) {
        return home_info.absoluteFilePath();
    }

    return QDir::currentPath();
}
}

QString FileDialogLocations::dialog_directory()
{
    QSettings settings;
    QString stored_path = settings.value(last_dialog_directory_key).toString();
    QFileInfo stored_info(stored_path);

    if (stored_info.isDir()) {
        return stored_info.absoluteFilePath();
    }

    return home_directory();
}

QString FileDialogLocations::start_path(const QString &file_name)
{
    QDir directory(dialog_directory());

    if (file_name.isEmpty()) {
        return directory.absolutePath();
    }

    return directory.filePath(file_name);
}

void FileDialogLocations::remember_directory(const QString &path)
{
    QFileInfo file_info(path);

    if (!file_info.isDir()) {
        return;
    }

    QSettings settings;
    settings.setValue(last_dialog_directory_key, file_info.absoluteFilePath());
}

void FileDialogLocations::remember_file_path(const QString &path)
{
    QFileInfo file_info(path);
    remember_directory(file_info.absolutePath());
}
