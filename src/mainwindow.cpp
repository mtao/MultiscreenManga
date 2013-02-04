#include "include/mainwindow.h"
#include "include/sidebar.h"
#include <QtGui/QDockWidget>
#include <QtGui/QMenuBar>
#include <QtGui/QFileDialog>
#include <QDebug>
#include <QKeyEvent>
#include <limits>

QString scanForHomePath(const QString &path) {
    if(path[0] == '~') {
        return QDir::homePath() + path.mid(1);
    } else {
        return path;
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_page_num(0)
    , m_root_dir(QDir::currentPath()) {
    m_fsmodel.setNameFilterDisables(false);
    setRoot(m_root_dir);

    config = std::make_shared<Configuration>();

    setMenuBar(new QMenuBar(this));
    QMenu *fileMenu = menuBar() -> addMenu(tr("&File"));
    QAction *openAct = new QAction(tr("&Open"), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(openRootVolume()));

    QAction *createRenderWidgetAct = new QAction(tr("&Render Window"), this);
    //    createRenderWidgetAct->setShortcuts(QKeySequence::Open);
    createRenderWidgetAct->setStatusTip(tr("New Render Window"));
    connect(createRenderWidgetAct, SIGNAL(triggered()),
            this, SLOT(createRenderWidget()));
    // Quit action
    QAction *quitAct = new QAction(tr("&Quit"), this);
    quitAct->setShortcuts(QKeySequence::Quit);
    quitAct->setStatusTip(tr("Quit"));
    connect(quitAct, SIGNAL(triggered()), this, SLOT(close()));

    fileMenu->addAction(openAct);
    fileMenu->addAction(createRenderWidgetAct);
    fileMenu->addAction(quitAct);


    m_control = new QDockWidget(tr("Control"), this);
    Sidebar * sidebar = new Sidebar(this);
    connect(sidebar, SIGNAL(filePathSelected(const QString &)),
            this, SLOT(openRootVolume(const QString &)));
    m_control->setWidget(sidebar);
    addDockWidget(Qt::LeftDockWidgetArea, m_control);


    RenderWidget * main_widget = createRenderWidget();
    main_widget->setParent(this);
    //    main_widget->setMain();

    setCentralWidget(main_widget);
}

void MainWindow::setFocus() {
    centralWidget()->setFocus();
}

RenderWidget * MainWindow::createRenderWidget() {
    m_renderer_mutex.lock();
    qWarning() << "New window: " << m_page_num << " " << m_renderwidgets.size();
    RenderWidget * widget = new RenderWidget(
                m_renderer_mutex
                , m_page_num
                , m_renderwidgets.size()
                , m_root_volume);
    connect(this, SIGNAL(newRootMangaVolume(std::shared_ptr< MangaVolume>)),
            widget, SLOT(setMangaVolume(std::shared_ptr< MangaVolume>)));
    connect(this, SIGNAL(newPage(uint)),
            widget, SLOT(setPage(uint)));
    connect(this, SIGNAL(closeAll()),
            widget, SLOT(close()));
    connect(widget, SIGNAL(renderWidgetClosing(uint)), this,
            SLOT(renderWidgetClosed(uint)));
    connect(widget, SIGNAL(passKeyPressEvent(QKeyEvent*)), this,
            SLOT(keyPressEvent(QKeyEvent*)));
    widget->show();
    m_renderwidgets.insert(std::unique_ptr<RenderWidgetResource>(
                               new RenderWidgetResource(widget, m_renderwidgets.size())));
    if(m_root_volume) {
        m_root_volume->getNumRenderWidgets(m_renderwidgets.size());
    }
    m_renderer_mutex.unlock();

    return widget;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    emit closeAll();
    QMainWindow::closeEvent(event);
}

void MainWindow::openRootVolume() {
    QFileDialog::Options options(QFileDialog::HideNameFilterDetails);
    QString selectedFilter("*");
    QString filename = QFileDialog::getOpenFileName(
                this,
                tr("Choose file or directory"),
                QDir::homePath(),
                config->getSupportedFileFilters(),
                &selectedFilter,
                options);

    if (filename.isNull()) {
        return;
    }
    openRootVolume(filename, true);
}

void MainWindow::openRootVolume(const QString & filepath, bool changeRoot) {
    QString path = scanForHomePath(filepath);
    QFileInfo fileinfo(path);
    if(changeRoot) {
        setRoot(fileinfo.dir().absolutePath());
    }
    bool willFindIndex = false;
    if(fileinfo.isFile() && config->isSupportedImageFormat(fileinfo.completeSuffix())) {
        path = fileinfo.dir().absolutePath();
        willFindIndex = true;
    }
    std::shared_ptr< MangaVolume> volume = openVolume(path);
    if (volume == NULL) {
        qWarning() << "Could not open path" << filepath;
        return;
    }
    m_root_volume.reset();
    if(volume->numPages() == 0) {
        return;
    }
    m_root_volume = volume;
    if(m_root_volume) {
        m_root_volume->getNumRenderWidgets(m_renderwidgets.size());
        m_curindex = m_fsmodel.index(path);
    }
    m_filename = fileinfo.fileName();//Now that we know taht we've opened the root vol set the filename...
    m_page_num = 0;
    emit newRootMangaVolume(m_root_volume);
    if(willFindIndex && m_root_volume) {
        qWarning() << "Finding index: " << m_root_volume->findIndex(path);
        emit changePage(m_root_volume->findIndex(filepath));
    }
}

std::shared_ptr<MangaVolume> MainWindow::openVolume(const QString & filename) {
    MangaVolume *volume;
    if (filename.endsWith(tr(".pdf"))) {
        volume = new PDFMangaVolume(filename, this);
    }
    else if (filename.endsWith(tr(".zip"))
             || filename.endsWith(tr(".rar"))
             || filename.endsWith(tr(".cbr"))
             || filename.endsWith(tr(".cbz"))
             ) {
        volume = new CompressedFileMangaVolume(filename, this);
    }
    else {
        QFileInfo fileInfo(filename);
        QString extension = fileInfo.completeSuffix();
        QString volPath = NULL;
        if (fileInfo.isDir()) {
            // If it's a directory, then use it as the volume root
            volPath = fileInfo.absoluteFilePath();
        } else if (config->isSupportedImageFormat(extension)) {
            // If it's a supported image file, use it's parent
            volPath = fileInfo.absolutePath();
        }

        if (volPath != NULL) {
            volume = new DirectoryMangaVolume(volPath, this);
        } else {
            qWarning() << "Specified path" << filename
                       << "is not a directory or recognized image format!";
            return NULL;
        }
    }

    std::shared_ptr<MangaVolume> volume_ptr(volume);
    return volume_ptr;
}

void MainWindow::nextPage() {
    changePage(m_page_num+m_renderwidgets.size());
}

void MainWindow::previousPage() {
    changePage(m_page_num-m_renderwidgets.size());
}

void MainWindow::changePage(int index) {
    if(!m_root_volume) {
        return;
    }
    // Enforce preconditions on page number
    if (index < 0) {
        changeVolume(-1);
        return;
        index = 0;
    }
    if (index >= m_root_volume->size()) {
        changeVolume(1);
        return;
        index = m_root_volume->size()-m_renderwidgets.size();
    }
    // Skip if we're already at the requested page
    if (m_page_num == index) {
        return;
    }
    // Change to the new page
    m_page_num = index;
    emit newPage(m_page_num);
}

void MainWindow::renderWidgetClosed(uint index) {
    bool need_to_unlock_myself = false;
    if (m_renderer_mutex.try_lock()) {
        qWarning() \
                << "Someone didn't lock mutex before starting index decrementing!" \
                << " I've locked it for myself now";
        need_to_unlock_myself = true;
    }

    auto it = m_renderwidgets.begin();
    int count = 0;
    for (; (*it)->index != index; ++count, ++it);
    it = m_renderwidgets.erase(it);
    for (; it != m_renderwidgets.end(); ++it) {
        (*it)->decrementIndex();
    }

    if(m_root_volume) {
        m_root_volume->getNumRenderWidgets(m_renderwidgets.size());
    }
    if (need_to_unlock_myself) {
        m_renderer_mutex.unlock();
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_Left:
    case Qt::Key_Up:
    case Qt::Key_PageUp:
        if(event->modifiers() & Qt::ShiftModifier) {
            nextPage();
        } else {
            previousPage();
        }
        break;
    case Qt::Key_Right:
    case Qt::Key_Down:
    case Qt::Key_PageDown:
    case Qt::Key_Space:
        if(event->modifiers() & Qt::ShiftModifier) {
            previousPage();
        } else {
            nextPage();
        }
        break;
    case Qt::Key_Home:
        changePage(0);
        break;
    case Qt::Key_End:
        changePage(std::numeric_limits<uint>::max());
        break;
    case Qt::Key_F:
        if (isFullScreen()) {
            showNormal();
            menuBar()->show();
        } else {
            showFullScreen();
            if (menuBar()->isHidden()) {
                menuBar()->show();
            } else {
                menuBar()->hide();
            }
        }
        break;
    case Qt::Key_Escape:
        if (isFullScreen()) {
            showNormal();
        }
        menuBar()->show();
    default:
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
    switch (event->button()) {
    case Qt::LeftButton:
        if (event->modifiers() & Qt::ShiftModifier) {
            previousPage();
        } else {
            nextPage();
        }
        break;
    default:
        QMainWindow::mousePressEvent(event);
    }
}

void MainWindow::wheelEvent(QWheelEvent *event) {
    if (event->delta() < 0) {
        nextPage();
    } else {
        previousPage();
    }
}

void MainWindow::setRoot(const QString & dirpath) {
    setRoot(QDir(scanForHomePath(dirpath)));

}
void MainWindow::setRoot(const QDir & dir) {
    m_root_dir = dir;
    m_fsmodel.setRootPath(dir.absolutePath());
    emit emitRootPath(dir.absolutePath());

}

void MainWindow::changeVolume(int index){
    QModelIndex newind = m_fsmodel.index(m_curindex.row()+index,m_curindex.column(),m_curindex.parent());
    if(newind.isValid())
    {

        openRootVolume(m_fsmodel.filePath(newind));
        if(m_root_volume && index < 0) {
            changePage(m_root_volume->size() - m_renderwidgets.size());
        }
    }
}

MainWindow::~MainWindow() {
}
