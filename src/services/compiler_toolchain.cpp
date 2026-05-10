#include "compiler_toolchain.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>

namespace {
constexpr const char *dev_version = "dev";
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

    if (result.active_version.isEmpty()) {
        result.available = false;
        result.message = "No platlang compiler has been installed yet.";
        return result;
    }

    result.compiler_path = compiler_path_for_version(result.active_version);
    result.available = QFileInfo::exists(result.compiler_path);

    if (result.available) {
        result.message = "Using platlang compiler " + result.active_version
                         + " from AppData.";
    } else {
        result.message = "Configured platlang compiler is missing: "
                         + result.compiler_path;
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
        result.message = "Selected compiler is not a file: " + source_path;
        return result;
    }

    QDir().mkpath(version_directory(version));
    QString target_path = compiler_path_for_version(version);
    QFile::remove(target_path);

    if (!QFile::copy(source_info.absoluteFilePath(), target_path)) {
        Status result = status();
        result.available = false;
        result.message = "Could not copy compiler to: " + target_path;
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
    return QDir(toolchain_root()).filePath("versions/" + version);
}

QString CompilerToolchain::compiler_path_for_version(const QString &version) const
{
    return QDir(version_directory(version)).filePath(compiler_file_name());
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
