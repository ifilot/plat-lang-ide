#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QIcon>

#include "main_window.h"
#include "theme_manager.h"

/**
 * Starts the plat-lang IDE application.
 *
 * @param argc Command-line argument count.
 * @param argv Command-line argument values.
 * @return Application exit code.
 */
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("plat-lang");
    QCoreApplication::setApplicationName("plat-lang IDE");
    app.setWindowIcon(QIcon(":/logos/plat-lang-logo.svg"));
    ThemeManager::apply_theme(app, ThemeManager::load_theme());

    QString startup_folder;
    QString startup_message;

    if (app.arguments().size() > 1) {
        QFileInfo file_info(app.arguments().at(1));

        if (file_info.isDir()) {
            startup_folder = file_info.absoluteFilePath();
        } else {
            startup_message = "Startup folder ignored because it is not a directory: "
                              + app.arguments().at(1);
        }
    } else {
        const QStringList candidate_example_paths = {
            QDir::current().absoluteFilePath("examples"),
            QDir(QCoreApplication::applicationDirPath())
                .absoluteFilePath("../examples"),
            QDir(QCoreApplication::applicationDirPath())
                .absoluteFilePath("../../examples")
        };

        for (const QString &path : candidate_example_paths) {
            QFileInfo file_info(QDir::cleanPath(path));

            if (file_info.isDir()) {
                startup_folder = file_info.absoluteFilePath();
                break;
            }
        }
    }

    MainWindow window(startup_folder, startup_message);
    window.show();

    return app.exec();
}
