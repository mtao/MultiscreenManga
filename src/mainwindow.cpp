#include "include/mainwindow.h"
#include <QtGui/QDockWidget>
#include <QtGui/QMenuBar>
#include <QtGui/QFileDialog>
#include <QDebug>
#include <QKeyEvent>
#include <limits>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_page_num(0) {

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
    connect(this, SIGNAL(newRootMangaVolume(std::shared_ptr<const MangaVolume>)),
            widget, SLOT(setMangaVolume(std::shared_ptr<const MangaVolume>)));
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
    m_renderer_mutex.unlock();

    return widget;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    emit closeAll();
    QMainWindow::closeEvent(event);
}

void MainWindow::openRootVolume() {
    QString filename = QFileDialog::getOpenFileName(0, tr("Choose file"), QString());
    // filename is null if user hit cancel
    if (filename.isNull()) {
        return;
    }
    openRootVolume(filename);
}

void MainWindow::openRootVolume(const QString & filepath) {
    std::shared_ptr<const MangaVolume> volume = openVolume(filepath);
    if (volume == NULL) {
        qWarning() << "Could not open path" << filepath;
        return;
    }
    m_root_volume.reset();
    m_root_volume = volume;
    m_page_num = 0;
    emit newRootMangaVolume(m_root_volume);
}

std::shared_ptr<const MangaVolume> MainWindow::openVolume(const QString & filename) {
    MangaVolume *volume;
    if (filename.endsWith(tr(".pdf"))) {
        volume = new PDFMangaVolume(filename, this);
    }
    else if (filename.endsWith(tr(".zip")) || filename.endsWith(tr(".rar"))) {
        volume = new CompressedFileMangaVolume(filename, this);
    }
    else {
        QFileInfo fileInfo(filename);
        QString extension = filename.split(".").last();
        QString volPath = NULL;
        if (fileInfo.isDir()) {
            // If it's a directory, then use it as the volume root
            volPath = fileInfo.absoluteFilePath();
        } else if (config->getSupportedImageFormats().contains(extension)) {
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
    std::shared_ptr<const MangaVolume> volume_ptr(volume);
    return volume_ptr;
}

void MainWindow::nextPage() {
    changePage(m_page_num+m_renderwidgets.size());
}

void MainWindow::previousPage() {
    changePage(m_page_num-m_renderwidgets.size());
}

void MainWindow::changePage(int index) {
    // Enforce preconditions on page number
    if (index < 0) {
        index = 0;
    }
    if (index >= m_root_volume->size()) {
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


    if (need_to_unlock_myself) {
        m_renderer_mutex.unlock();
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_Left:
    case Qt::Key_Up:
    case Qt::Key_PageUp:
        previousPage();
        break;
    case Qt::Key_Right:
    case Qt::Key_Down:
    case Qt::Key_PageDown:
    case Qt::Key_Space:
        nextPage();
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


MainWindow::~MainWindow() {
}
