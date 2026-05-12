#ifndef COMPILER_TOOLCHAIN_H
#define COMPILER_TOOLCHAIN_H

#include <QByteArray>
#include <QString>

/**
 * Manages the locally installed platlang compiler toolchain.
 */
class CompilerToolchain {
public:
    /**
     * Selects where automatic interpreter updates are fetched from.
     */
    enum class UpdateChannel {
        StableRelease,
        DevelopBranch
    };

    /**
     * Describes the current toolchain resolution result.
     */
    struct Status {
        QString compiler_path;
        QString active_version;
        QString storage_root;
        QString message;
        UpdateChannel update_channel;
        bool available;
    };

    /**
     * Creates a compiler toolchain manager.
     */
    CompilerToolchain();

    /**
     * Ensures the local toolchain storage exists and development compiler is
     * available when present.
     *
     * @return Current toolchain status.
     */
    Status initialize();

    /**
     * Returns the current toolchain status.
     *
     * @return Toolchain status.
     */
    Status status() const;

    /**
     * Installs a compiler executable into local toolchain storage.
     *
     * @param source_path Compiler executable selected by the user.
     * @param version Version name to store.
     * @return Current toolchain status after installation.
     */
    Status install_compiler(const QString &source_path,
                            const QString &version = "custom");

    /**
     * Installs compiler executable bytes into local toolchain storage.
     *
     * @param compiler_data Compiler executable contents.
     * @param version Semver tag to store.
     * @return Current toolchain status after installation.
     */
    Status install_compiler_data(const QByteArray &compiler_data,
                                 const QString &version);

    /**
     * Loads the selected update channel.
     *
     * @return Saved update channel.
     */
    static UpdateChannel load_update_channel();

    /**
     * Saves the selected update channel.
     *
     * @param channel Update channel to use.
     */
    static void save_update_channel(UpdateChannel channel);

    /**
     * Returns the stored version id for a develop branch commit.
     *
     * @param sha Full Git commit SHA.
     * @return Develop version id.
     */
    static QString develop_version(const QString &sha);

private:
    /**
     * Returns the executable name for the current platform.
     *
     * @return Compiler executable file name.
     */
    QString compiler_file_name() const;

    /**
     * Returns the AppData root used for toolchains.
     *
     * @return Toolchain storage root.
     */
    QString toolchain_root() const;

    /**
     * Returns the versioned directory for a compiler version.
     *
     * @param version Toolchain version.
     * @return Version directory path.
     */
    QString version_directory(const QString &version) const;

    /**
     * Returns the compiler path stored for a version.
     *
     * @param version Toolchain version.
     * @return Compiler executable path.
     */
    QString compiler_path_for_version(const QString &version) const;

    /**
     * Returns the configured active compiler version.
     *
     * @return Active version string.
     */
    QString active_version() const;

    /**
     * Stores the active compiler version.
     *
     * @param version Active version string.
     */
    void set_active_version(const QString &version) const;

    /**
     * Copies the development compiler into AppData when it exists.
     *
     * @return True when the development compiler is available in AppData.
     */
    bool ensure_development_toolchain() const;

    /**
     * Returns the known development compiler path when it exists.
     *
     * @return Absolute compiler path, or empty string.
     */
    QString find_development_compiler() const;

    /**
     * Ensures a copied compiler can be executed.
     *
     * @param path Compiler path.
     */
    void make_executable(const QString &path) const;
};

#endif
