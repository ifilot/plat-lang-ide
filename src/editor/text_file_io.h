#ifndef TEXT_FILE_IO_H
#define TEXT_FILE_IO_H

#include <QString>

#include <optional>

namespace TextFileIo {

/**
 * Reads a UTF-8 text file from disk.
 *
 * @param path File path to read.
 * @param error Optional destination for a user-facing error.
 * @return File contents when read successfully.
 */
std::optional<QString> read_utf8_file(const QString &path,
                                      QString *error = nullptr);

/**
 * Writes UTF-8 text to disk, replacing any existing file.
 *
 * @param path File path to write.
 * @param contents Text contents.
 * @param error Optional destination for a user-facing error.
 * @return True when the file was written.
 */
bool write_utf8_file(const QString &path, const QString &contents,
                     QString *error = nullptr);

/**
 * Returns whether a file appears to contain binary data.
 *
 * @param path File path to inspect.
 * @return True when the file should be treated as binary.
 */
bool is_binary_file(const QString &path);

}

#endif
