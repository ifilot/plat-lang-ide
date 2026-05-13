#include "compiler_toolchain.h"

#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>
#include <QTemporaryDir>

#include <archive.h>
#include <archive_entry.h>

namespace {
constexpr const char *dev_version = "dev";
constexpr const char *develop_version_prefix = "develop-";
constexpr const char *update_channel_setting_key =
    "toolchains/platlang/update_channel";

QString translate_toolchain(const char *text)
{
    return QCoreApplication::translate("CompilerToolchain", text);
}

QString archive_error_message(archive *archive_handle)
{
    const char *error = archive_error_string(archive_handle);
    return error == nullptr ? QString{} : QString::fromLocal8Bit(error);
}
}

CompilerToolchain::CompilerToolchain()
{
}

CompilerToolchain::Status CompilerToolchain::initialize()
{
    QDir().mkpath(toolchain_root());
    QDir().mkpath(version_directory(dev_version));

    bool has_dev_toolchain = ensure_development_toolchain();
    QString configured_version = active_version();

    if (configured_version.isEmpty() && has_dev_toolchain) {
        set_active_version(dev_version);
        configured_version = dev_version;
    }

    return status();
}

CompilerToolchain::Status CompilerToolchain::status() const
{
    Status result;
    result.storage_root = toolchain_root();
    result.active_version = active_version();
    result.update_channel = load_update_channel();

    if (result.active_version.isEmpty()) {
        result.available = false;
        result.message =
            translate_toolchain("No platlang compiler has been installed yet.");
        return result;
    }

    result.compiler_path = compiler_path_for_version(result.active_version);
    result.available = QFileInfo::exists(result.compiler_path);

    if (result.available) {
        result.message =
            QCoreApplication::translate(
                "CompilerToolchain",
                "Using platlang compiler %1 from AppData.")
                .arg(result.active_version);
    } else {
        result.message =
            QCoreApplication::translate(
                "CompilerToolchain",
                "Configured platlang compiler is missing: %1")
                .arg(result.compiler_path);
    }

    return result;
}

CompilerToolchain::Status CompilerToolchain::install_compiler(
    const QString &source_path, const QString &version)
{
    QFileInfo source_info(source_path);

    if (!source_info.isFile()) {
        Status result = status();
        result.available = false;
        result.message =
            QCoreApplication::translate(
                "CompilerToolchain",
                "Selected compiler is not a file: %1")
                .arg(source_path);
        return result;
    }

    QDir().mkpath(version_directory(version));
    QString target_path = compiler_path_for_version(version);
    QFile::remove(target_path);

    if (!QFile::copy(source_info.absoluteFilePath(), target_path)) {
        Status result = status();
        result.available = false;
        result.message =
            QCoreApplication::translate(
                "CompilerToolchain",
                "Could not copy compiler to: %1")
                .arg(target_path);
        return result;
    }

    make_executable(target_path);
    set_active_version(version);
    return status();
}

CompilerToolchain::UpdateChannel CompilerToolchain::load_update_channel()
{
    QSettings settings;
    QString value = settings.value(update_channel_setting_key, "stable").toString();

    if (value == "develop") {
        return UpdateChannel::DevelopBranch;
    }

    return UpdateChannel::StableRelease;
}

void CompilerToolchain::save_update_channel(UpdateChannel channel)
{
    QSettings settings;
    settings.setValue(update_channel_setting_key,
                      channel == UpdateChannel::DevelopBranch
                          ? "develop"
                          : "stable");
}

QString CompilerToolchain::develop_version(const QString &sha,
                                           const QString &asset_identity)
{
    QString version = QString(develop_version_prefix) + sha;

    if (asset_identity.isEmpty()) {
        return version;
    }

    QString safe_asset_identity;
    safe_asset_identity.reserve(asset_identity.size());

    for (const QChar &character : asset_identity) {
        safe_asset_identity.append(character.isLetterOrNumber()
                                       ? character
                                       : QLatin1Char('-'));
    }

    if (!safe_asset_identity.isEmpty()) {
        version += QStringLiteral("-asset-") + safe_asset_identity.left(24);
    }

    return version;
}

CompilerToolchain::Status CompilerToolchain::install_compiler_data(
    const QByteArray &compiler_data, const QString &version,
    const QString &asset_name)
{
    if (compiler_data.isEmpty()) {
        Status result = status();
        result.available = false;
        result.message = translate_toolchain("Downloaded compiler was empty.");
        return result;
    }

    if (is_compiler_package(asset_name)) {
        return install_compiler_package_data(compiler_data, version, asset_name);
    }

    QDir().mkpath(version_directory(version));
    QString target_path = compiler_path_for_version(version);
    QFile::remove(target_path);

    QFile target_file(target_path);

    if (!target_file.open(QIODevice::WriteOnly)) {
        Status result = status();
        result.available = false;
        result.message =
            QCoreApplication::translate(
                "CompilerToolchain",
                "Could not write compiler to: %1")
                .arg(target_path);
        return result;
    }

    qint64 bytes_written = target_file.write(compiler_data);
    target_file.close();

    if (bytes_written != compiler_data.size()) {
        QFile::remove(target_path);
        Status result = status();
        result.available = false;
        result.message =
            QCoreApplication::translate(
                "CompilerToolchain",
                "Could not finish writing compiler to: %1")
                .arg(target_path);
        return result;
    }

    make_executable(target_path);
    set_active_version(version);
    return status();
}

QString CompilerToolchain::compiler_file_name() const
{
#ifdef Q_OS_WIN
    return "platlang.exe";
#else
    return "platlang";
#endif
}

QString CompilerToolchain::toolchain_root() const
{
    QString app_data = QStandardPaths::writableLocation(
        QStandardPaths::AppDataLocation);
    return QDir(app_data).filePath("toolchains/platlang");
}

QString CompilerToolchain::version_directory(const QString &version) const
{
    if (version.startsWith(develop_version_prefix)) {
        return QDir(toolchain_root())
            .filePath("develop/" + version.mid(QString(develop_version_prefix).size()));
    }

    return QDir(toolchain_root()).filePath("versions/" + version);
}

QString CompilerToolchain::compiler_path_for_version(const QString &version) const
{
    QDir directory(version_directory(version));
    const QString root_compiler_path = directory.filePath(compiler_file_name());

    if (QFileInfo::exists(root_compiler_path)) {
        return root_compiler_path;
    }

    const QString bin_compiler_path =
        directory.filePath(QStringLiteral("bin/") + compiler_file_name());

    if (QFileInfo::exists(bin_compiler_path)) {
        return bin_compiler_path;
    }

    return root_compiler_path;
}

QString CompilerToolchain::active_version() const
{
    QSettings settings;
    return settings.value("toolchains/platlang/active_version").toString();
}

void CompilerToolchain::set_active_version(const QString &version) const
{
    QSettings settings;
    settings.setValue("toolchains/platlang/active_version", version);
}

bool CompilerToolchain::ensure_development_toolchain() const
{
    QString source_path = find_development_compiler();

    if (source_path.isEmpty()) {
        return false;
    }

    QString target_path = compiler_path_for_version(dev_version);
    QFileInfo source_info(source_path);
    QFileInfo target_info(target_path);

    bool should_copy = !target_info.exists()
                       || source_info.lastModified() > target_info.lastModified()
                       || source_info.size() != target_info.size();

    if (!should_copy) {
        return true;
    }

    QDir().mkpath(version_directory(dev_version));
    QFile::remove(target_path);

    if (!QFile::copy(source_path, target_path)) {
        return false;
    }

    make_executable(target_path);
    return true;
}

QString CompilerToolchain::find_development_compiler() const
{
    const QString file_name = compiler_file_name();
    const QStringList candidate_paths = {
        QDir::current().absoluteFilePath("../../platlang/build/" + file_name),
        QDir(QCoreApplication::applicationDirPath())
            .absoluteFilePath("../../platlang/build/" + file_name),
        QDir(QCoreApplication::applicationDirPath())
            .absoluteFilePath("../platlang/build/" + file_name)
    };

    for (const QString &path : candidate_paths) {
        QFileInfo file_info(QDir::cleanPath(path));

        if (file_info.isFile()) {
            return file_info.absoluteFilePath();
        }
    }

    return {};
}

CompilerToolchain::Status CompilerToolchain::install_compiler_package_data(
    const QByteArray &package_data, const QString &version,
    const QString &asset_name)
{
    QTemporaryDir temporary_directory;

    if (!temporary_directory.isValid()) {
        Status result = status();
        result.available = false;
        result.message =
            translate_toolchain("Could not create a temporary compiler package directory.");
        return result;
    }

    const QString extract_path =
        QDir(temporary_directory.path()).filePath("extracted");
    QDir().mkpath(extract_path);

    archive *reader = archive_read_new();
    archive_read_support_filter_all(reader);
    archive_read_support_format_all(reader);

    int archive_result = archive_read_open_memory(
        reader, package_data.constData(),
        static_cast<size_t>(package_data.size()));

    if (archive_result != ARCHIVE_OK) {
        QString details = archive_error_message(reader);
        archive_read_free(reader);

        Status result = status();
        result.available = false;
        result.message =
            QCoreApplication::translate(
                "CompilerToolchain",
                "Could not read compiler package %1.")
                .arg(asset_name)
            + (details.isEmpty() ? QString{} : QStringLiteral(" ") + details);
        return result;
    }

    archive_entry *entry = nullptr;

    while ((archive_result = archive_read_next_header(reader, &entry))
           == ARCHIVE_OK) {
        QString entry_path = QString::fromUtf8(archive_entry_pathname(entry));
        entry_path.replace('\\', '/');
        entry_path = QDir::cleanPath(entry_path);

        if (entry_path.isEmpty()
            || QDir::isAbsolutePath(entry_path)
            || entry_path == ".."
            || entry_path.startsWith("../")) {
            archive_read_free(reader);

            Status result = status();
            result.available = false;
            result.message =
                QCoreApplication::translate(
                    "CompilerToolchain",
                    "Compiler package %1 contains an unsafe path.")
                    .arg(asset_name);
            return result;
        }

        const QString output_path = QDir(extract_path).filePath(entry_path);
        const auto file_type = archive_entry_filetype(entry);

        if (file_type == AE_IFDIR) {
            QDir().mkpath(output_path);
            continue;
        }

        if (file_type != AE_IFREG) {
            archive_read_data_skip(reader);
            continue;
        }

        QDir().mkpath(QFileInfo(output_path).absolutePath());
        QFile output_file(output_path);

        if (!output_file.open(QIODevice::WriteOnly)) {
            archive_read_free(reader);

            Status result = status();
            result.available = false;
            result.message =
                QCoreApplication::translate(
                    "CompilerToolchain",
                    "Could not write extracted compiler package file: %1")
                    .arg(output_path);
            return result;
        }

        const void *block = nullptr;
        size_t block_size = 0;
        la_int64_t block_offset = 0;

        while ((archive_result = archive_read_data_block(
                    reader, &block, &block_size, &block_offset))
               == ARCHIVE_OK) {
            if (!output_file.seek(block_offset)
                || output_file.write(static_cast<const char *>(block),
                                     static_cast<qint64>(block_size))
                       != static_cast<qint64>(block_size)) {
                output_file.close();
                archive_read_free(reader);

                Status result = status();
                result.available = false;
                result.message =
                    QCoreApplication::translate(
                        "CompilerToolchain",
                        "Could not write extracted compiler package file: %1")
                        .arg(output_path);
                return result;
            }
        }

        output_file.close();

        if (archive_result != ARCHIVE_EOF) {
            QString details = archive_error_message(reader);
            archive_read_free(reader);

            Status result = status();
            result.available = false;
            result.message =
                QCoreApplication::translate(
                    "CompilerToolchain",
                    "Could not extract compiler package %1.")
                    .arg(asset_name)
                + (details.isEmpty() ? QString{} : QStringLiteral(" ") + details);
            return result;
        }
    }

    if (archive_result != ARCHIVE_EOF) {
        QString details = archive_error_message(reader);
        archive_read_free(reader);

        Status result = status();
        result.available = false;
        result.message =
            QCoreApplication::translate(
                "CompilerToolchain",
                "Could not extract compiler package %1.")
                .arg(asset_name)
            + (details.isEmpty() ? QString{} : QStringLiteral(" ") + details);
        return result;
    }

    archive_read_free(reader);

    const QString extracted_compiler = find_extracted_compiler(extract_path);

    if (extracted_compiler.isEmpty()) {
        Status result = status();
        result.available = false;
        result.message =
            QCoreApplication::translate(
                "CompilerToolchain",
                "Compiler package %1 did not contain a platlang executable.")
                .arg(asset_name);
        return result;
    }

    const QString target_directory = version_directory(version);
    QDir(target_directory).removeRecursively();
    QDir().mkpath(target_directory);

    const QString package_root =
        extracted_package_root(extract_path, extracted_compiler);

    if (!copy_directory_contents(package_root, target_directory)) {
        Status result = status();
        result.available = false;
        result.message =
            QCoreApplication::translate(
                "CompilerToolchain",
                "Could not copy compiler package files to: %1")
                .arg(target_directory);
        return result;
    }

    const QString target_compiler = compiler_path_for_version(version);

    if (!QFileInfo::exists(target_compiler)) {
        Status result = status();
        result.available = false;
        result.message =
            QCoreApplication::translate(
                "CompilerToolchain",
                "Compiler package %1 did not install a platlang executable.")
                .arg(asset_name);
        return result;
    }

    make_executable(target_compiler);
    set_active_version(version);
    return status();
}

bool CompilerToolchain::is_compiler_package(const QString &asset_name) const
{
    const QString lower_name = asset_name.toLower();
    return lower_name.endsWith(".tar")
           || lower_name.endsWith(".tar.gz")
           || lower_name.endsWith(".tgz")
           || lower_name.endsWith(".zip");
}

QString CompilerToolchain::find_extracted_compiler(const QString &root) const
{
    const QString expected_file_name = compiler_file_name();
    QString fallback_path;
    QDirIterator iterator(root, QDir::Files, QDirIterator::Subdirectories);

    while (iterator.hasNext()) {
        const QString path = iterator.next();
        QFileInfo file_info(path);
        const QString file_name = file_info.fileName();
        const QString lower_name = file_name.toLower();

        if (file_name == expected_file_name) {
            return file_info.absoluteFilePath();
        }

#ifdef Q_OS_WIN
        if (fallback_path.isEmpty()
            && lower_name.contains("platlang")
            && lower_name.endsWith(".exe")) {
            fallback_path = file_info.absoluteFilePath();
        }
#else
        if (fallback_path.isEmpty()
            && lower_name.contains("platlang")
            && !lower_name.endsWith(".exe")) {
            fallback_path = file_info.absoluteFilePath();
        }
#endif
    }

    return fallback_path;
}

QString CompilerToolchain::extracted_package_root(
    const QString &extracted_root, const QString &compiler_path) const
{
    Q_UNUSED(compiler_path);

    QDir root_directory(extracted_root);
    const QFileInfoList entries = root_directory.entryInfoList(
        QDir::NoDotAndDotDot | QDir::AllEntries);

    if (entries.size() == 1 && entries.first().isDir()) {
        return entries.first().absoluteFilePath();
    }

    return extracted_root;
}

bool CompilerToolchain::copy_directory_contents(
    const QString &source_root, const QString &target_root) const
{
    QDir source_directory(source_root);
    QDirIterator iterator(source_root, QDir::Files,
                          QDirIterator::Subdirectories);

    while (iterator.hasNext()) {
        const QString source_path = iterator.next();
        const QString relative_path = source_directory.relativeFilePath(source_path);
        const QString target_path = QDir(target_root).filePath(relative_path);

        if (!QDir().mkpath(QFileInfo(target_path).absolutePath())) {
            return false;
        }

        QFile::remove(target_path);

        if (!QFile::copy(source_path, target_path)) {
            return false;
        }

        QFile target_file(target_path);
        target_file.setPermissions(QFile(source_path).permissions());
    }

    return true;
}

void CompilerToolchain::make_executable(const QString &path) const
{
    QFile file(path);
    QFileDevice::Permissions permissions = file.permissions();
    permissions |= QFileDevice::ExeOwner
                   | QFileDevice::ExeUser
                   | QFileDevice::ExeGroup
                   | QFileDevice::ExeOther;
    file.setPermissions(permissions);
}
