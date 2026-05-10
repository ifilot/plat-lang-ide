#include "text_file_io.h"

#include <QFile>
#include <QIODevice>

namespace TextFileIo {

std::optional<QString> read_utf8_file(const QString &path, QString *error)
{
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (error != nullptr) {
            *error = "Could not open " + path + ".";
        }

        return std::nullopt;
    }

    return QString::fromUtf8(file.readAll());
}

bool write_utf8_file(const QString &path, const QString &contents,
                     QString *error)
{
    QFile file(path);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        if (error != nullptr) {
            *error = "Could not save " + path + ".";
        }

        return false;
    }

    file.write(contents.toUtf8());
    return true;
}

bool is_binary_file(const QString &path)
{
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly)) {
        return true;
    }

    QByteArray sample = file.read(4096);

    for (char byte : sample) {
        unsigned char value = static_cast<unsigned char>(byte);

        if (value == 0) {
            return true;
        }

        bool is_text_control = value == '\n' || value == '\r' || value == '\t';

        if (value < 32 && !is_text_control) {
            return true;
        }
    }

    return false;
}

}
