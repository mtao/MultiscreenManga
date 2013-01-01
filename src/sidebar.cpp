#include "include/sidebar.h"
#include "include/configuration.h"
#include <QFileSystemModel>
#include <QVBoxLayout>
#include <QDebug>
FileViewer::FileViewer(QWidget * parent ): QTreeView(parent) {

}

void Sidebar::modelItemSelected(const QModelIndex & index) {
    emit filePathSelected(model->filePath(index));
}


Sidebar::Sidebar(QWidget * parent): QWidget(parent) {
    QVBoxLayout * layout = new QVBoxLayout(this);
    setLayout(layout);
    model = new QFileSystemModel(this);
    model->setResolveSymlinks(false);
    qWarning() << Configuration().getSupportedFileFilters();
    model->setNameFilters(
                Configuration().getSupportedFileFiltersList()
                          );
    model->setNameFilterDisables(false);
    model->setRootPath(QDir::currentPath());
    QTreeView *tree = new QTreeView(this);
    tree->setModel(model);
    tree->setSortingEnabled(true);
    tree->setRootIndex(model->index(QDir::currentPath()));
    connect(
                tree, SIGNAL(doubleClicked(const QModelIndex &)),
                this, SLOT(modelItemSelected(const QModelIndex &))
            );
    layout->addWidget(tree);

}
