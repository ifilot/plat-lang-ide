#include "recent_files.h"

#include <QFileInfo>
#include <QSettings>

namespace {
constexpr const char *recent_files_key = "files/recent";
constexpr int max_recent_files = 10;

QString canonical_file_path(const QString &path)
{
    QFileInfo file_info(path);

    if (!file_info.isFile()) {
        return {};
    }

    QString canonical_path = file_info.canonicalFilePath();

    if (!canonical_path.isEmpty()) {
        return canonical_path;
    }

    return file_info.absoluteFilePath();
}

void save_files(const QStringList &files)
{
    QSettings settings;
    settings.setValue(recent_files_key, files);
}
}

QStringList RecentFiles::files()
{
    QSettings settings;
    QStringList stored_files = settings.value(recent_files_key).toStringList();
    QStringList existing_files;

    for (const QString &path : stored_files) {
        QString canonical_path = canonical_file_path(path);

        if (canonical_path.isEmpty() || existing_files.contains(canonical_path)) {
            continue;
        }

        existing_files.append(canonical_path);
    }

    if (existing_files != stored_files) {
        save_files(existing_files);
    }

    return existing_files;
}

void RecentFiles::remember_file(const QString &path)
{
    QString canonical_path = canonical_file_path(path);

    if (canonical_path.isEmpty()) {
        return;
    }

    QStringList recent_files = files();
    recent_files.removeAll(canonical_path);
    recent_files.prepend(canonical_path);

    while (recent_files.size() > max_recent_files) {
        recent_files.removeLast();
    }

    save_files(recent_files);
}

void RecentFiles::forget_file(const QString &path)
{
    QString canonical_path = canonical_file_path(path);
    QStringList recent_files = files();

    if (canonical_path.isEmpty()) {
        recent_files.removeAll(path);
    } else {
        recent_files.removeAll(canonical_path);
    }

    save_files(recent_files);
}
