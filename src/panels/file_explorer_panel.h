#ifndef FILE_EXPLORER_PANEL_H
#define FILE_EXPLORER_PANEL_H

#include <QString>
#include <QWidget>

class QFileSystemModel;
class QLabel;
class QModelIndex;
class QTreeView;

/**
 * Displays the project file tree.
 */
class FileExplorerPanel : public QWidget {
    Q_OBJECT

public:
    /**
     * Creates a file explorer rooted at the current working directory.
     *
     * @param parent Optional parent widget.
     */
    explicit FileExplorerPanel(QWidget *parent = nullptr);

    /**
     * Sets the root directory displayed by the explorer.
     *
     * @param path Directory path to display.
     * @return True when the explorer root was changed.
     */
    bool set_root_directory(const QString &path);

    /**
     * Returns the directory currently used as explorer root.
     *
     * @return Current root directory.
     */
    QString root_directory() const;

signals:
    /**
     * Emitted when a file is selected for opening.
     *
     * @param path Absolute path to the selected file.
     */
    void file_open_requested(const QString &path);

private slots:
    /**
     * Handles clicks in the file tree.
     *
     * @param index Clicked model index.
     */
    void handle_tree_clicked(const QModelIndex &index);

private:
    QFileSystemModel *model_;
    QLabel *root_label_;
    QTreeView *tree_view_;
};

#endif
