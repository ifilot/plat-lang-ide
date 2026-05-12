#include "file_explorer_panel.h"

#include <QDir>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QHeaderView>
#include <QLabel>
#include <QModelIndex>
#include <QTreeView>
#include <QVBoxLayout>

#include "app_fonts.h"
#include "file_icon_provider.h"

FileExplorerPanel::FileExplorerPanel(QWidget *parent)
    : QWidget(parent),
      model_(new QFileSystemModel(this)),
      root_label_(new QLabel(this)),
      tree_view_(new QTreeView(this))
{
    model_->setIconProvider(new FileIconProvider());

    QFont root_font = root_label_->font();
    root_font.setBold(true);
    root_label_->setFont(root_font);
    root_label_->setContentsMargins(8, 6, 8, 4);
    root_label_->setTextInteractionFlags(Qt::TextSelectableByMouse);

    tree_view_->setModel(model_);
    tree_view_->setFont(AppFonts::code_font(10));
    set_root_directory(QDir::currentPath());
    tree_view_->setAnimated(true);
    tree_view_->setIconSize(QSize(16, 16));
    tree_view_->setIndentation(16);
    tree_view_->setSortingEnabled(true);
    tree_view_->sortByColumn(0, Qt::AscendingOrder);
    tree_view_->header()->hide();
    tree_view_->header()->setStretchLastSection(false);
    tree_view_->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    tree_view_->hideColumn(1);
    tree_view_->hideColumn(2);
    tree_view_->hideColumn(3);

    connect(tree_view_, &QTreeView::clicked,
            this, &FileExplorerPanel::handle_tree_clicked);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(root_label_);
    layout->addWidget(tree_view_);
}

bool FileExplorerPanel::set_root_directory(const QString &path)
{
    QFileInfo file_info(path);

    if (!file_info.isDir()) {
        return false;
    }

    QString root_path = file_info.absoluteFilePath();
    QModelIndex root_index = model_->setRootPath(root_path);
    QString root_name = file_info.fileName();

    if (root_name.isEmpty()) {
        root_name = QDir(root_path).dirName();
    }

    if (root_name.isEmpty()) {
        root_name = root_path;
    }

    root_label_->setText(root_name);
    root_label_->setToolTip(root_path);

    if (tree_view_->model() == model_) {
        tree_view_->setRootIndex(root_index);
    }

    return true;
}

QString FileExplorerPanel::root_directory() const
{
    return model_->rootPath();
}

void FileExplorerPanel::handle_tree_clicked(const QModelIndex &index)
{
    if (!index.isValid() || model_->isDir(index)) {
        return;
    }

    emit file_open_requested(model_->filePath(index));
}
