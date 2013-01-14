#include "include/sidebar.h"
#include "include/configuration.h"
#include <QFileSystemModel>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QMenu>
#include <QFileDialog>
#include <QHeaderView>

FileViewer::FileViewer(QWidget * parent ): QTreeView(parent) {
    QFileSystemModel * model = new QFileSystemModel(this);
    model->setResolveSymlinks(false);
    model->setNameFilters(
                Configuration().getSupportedFileFiltersList()
                );
    model->setNameFilterDisables(false);
    model->setRootPath(QDir::currentPath());
    header()->setMovable(true);
    this->setModel(model);
    this->setSortingEnabled(true);
    this->setRootIndex(model->index(QDir::currentPath()));
    this->header()->swapSections(1,3);
    this->header()->hideSection(2);//type
    this->header()->setResizeMode(QHeaderView::Interactive);
    this->header()->resizeSections(QHeaderView::ResizeToContents);



}

void FileViewer::mousePressEvent(QMouseEvent * event) {
    if (event->button() == Qt::RightButton) {
        QMenu * menu = new QMenu(this);
        QAction * setRootAction = new QAction(tr("New &Root"), this);
        connect(setRootAction, SIGNAL(triggered()), this, SLOT(selectRoot()) );
        menu->addAction(setRootAction);
        menu->exec(mapToGlobal(event->pos()));
        event->accept();
    }
    QTreeView::mousePressEvent(event);

}

void FileViewer::selectRoot() {
    setRoot(QFileDialog::getExistingDirectory(
                this,
                tr("Choose new root directory"),
                QDir::currentPath()
                )
            );


}

void FileViewer::setRoot(const QString & dirname) {
    if (dirname.isNull()) {
        return;
    } else {
        QFileSystemModel* model = static_cast<QFileSystemModel *>(this->model());
        setRootIndex(model->index(dirname));
        model->setRootPath(dirname);
        this->header()->resizeSections(QHeaderView::ResizeToContents);
    }
}

void Sidebar::modelItemSelected(const QModelIndex & index) {
    emit filePathSelected(static_cast<QFileSystemModel *>(tree->model())->filePath(index));
}

Sidebar::Sidebar(QWidget * parent): QWidget(parent), tree(0) {
    QVBoxLayout * layout = new QVBoxLayout(this);
    setLayout(layout);
    tree = new FileViewer(this);

    connect(
                tree, SIGNAL(doubleClicked(const QModelIndex &)),
                this, SLOT(modelItemSelected(const QModelIndex &))
                );
    connect(parent, SIGNAL(emitRootPath(const QString &)), tree, SLOT(setRoot(const QString &)));
    layout->addWidget(tree);

}
