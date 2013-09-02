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
    m_fsmodel.setResolveSymlinks(false);
    m_fsmodel.setNameFilters(
                Configuration().getSupportedFileFiltersList()
                );
    m_fsmodel.setNameFilterDisables(false);
    m_fsmodel.sort(0,Qt::AscendingOrder);
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
    Sidebar * sidebar = new Sidebar(&m_fsmodel,this);
    connect(sidebar, SIGNAL(filePathSelected(const QString &)),
            this, SLOT(openRootVolume(const QString &)));
    m_control->setWidget(sidebar);
    addDockWidget(Qt::LeftDockWidgetArea, m_control);


    RenderWidget * main_widget = createRenderWidget();
    main_widget->setParent(this);
    //    main_widget->setMain();

    initializeKeyBindings();
    setCentralWidget(main_widget);
    main_widget->setFocus(Qt::ActiveWindowFocusReason);
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

KeyMappableFunction::ptr& MainWindow::addKeyMappableFunction(KeyMappableFunction::ptr&& ptr) {
    return (m_available_keys.emplace(std::make_pair(ptr->short_name,ptr)).first)->second;
}
KeyMappableFunction::ptr& MainWindow::addKeyMappableFunction(QString&& short_name, QString&& name, std::function<void(void)>&& f) {
    return addKeyMappableFunction(std::make_shared<KeyMappableFunction>(short_name,name,f));
}

void MainWindow::initializeKeyBindings() {

    /*
       case Qt::Key_Right:
       case Qt::Key_Down:
       case Qt::Key_PageDown:
       case Qt::Key_Space:
       */
    auto&& next_page = addKeyMappableFunction("NextPage", "Next Page",
            [&]() {
            nextPage();
            });
    /*
       case Qt::Key_Left:
       case Qt::Key_Up:
       case Qt::Key_PageUp:
       */
    auto&& prev_page = addKeyMappableFunction("PrevPage", "Previous Page",
            [&]() {
            previousPage();
            });
    /*
       case Qt::Key_Home:
       */
    auto&& first_page = addKeyMappableFunction("FirstPage", "First Page",
            [&]() {
            changePage(0);
            });
    /*
       case Qt::Key_End:
       */
    auto&& last_page = addKeyMappableFunction("LastPage", "Last Page",
            [&]() {
            if(m_root_volume)
            {
            changePage(m_root_volume->size()-m_renderwidgets.size());
            }
            });
    /*
       case Qt::Key_F:
       */
    auto&& fullscreen = addKeyMappableFunction("Fullscreen", "Fullscreen",
            [&]() {
            //Letting qt manage full screen state is bad on xmonad so I'll manually manage
            showBars = !showBars;
            if(showBars) {
            showNormal();
            toggleBars(true);

            } else {
            showFullScreen();
            toggleBars(false);
            }
            });
    /*
       case Qt::Key_Escape:
       */
    auto&& not_fullscreen = addKeyMappableFunction("!Fullscreen", "Leave fullscreen",
            [&](){
            showNormal();
            toggleBars(true);
            showBars=true;
            });
    for(int k: {Qt::Key_Right, Qt::Key_Down, Qt::Key_PageDown, Qt::Key_Space}) {
    m_keys.insert(std::make_pair(k,next_page));
    m_keys.insert(std::make_pair(k|Qt::ShiftModifier,prev_page));
    }
    for(int k: {Qt::Key_Left, Qt::Key_Up, Qt::Key_PageUp}) {
    m_keys.insert(std::make_pair(k,prev_page));
    m_keys.insert(std::make_pair(k|Qt::ShiftModifier,next_page));
    }
    m_keys.insert(std::make_pair(Qt::Key_F, fullscreen));
    m_keys.insert(std::make_pair(Qt::Key_Escape, not_fullscreen));
    m_keys.insert(std::make_pair(Qt::Key_Home, first_page));
    m_keys.insert(std::make_pair(Qt::Key_End, last_page));

}

void MainWindow::toggleBars(bool show) {
    if(show) {
        qWarning() << "Showing bars";
            menuBar()->show();
            m_control->show();
    } else {
        qWarning() << "Hiding bars";
            menuBar()->hide();
            m_control->hide();
    }

}


void MainWindow::openRootVolume(const QString & filepath, bool changeRoot) {
    QString path = scanForHomePath(filepath);
    QFileInfo fileinfo(path);
    if(changeRoot) {
        setRoot(fileinfo.dir().absolutePath());
    }
    bool willFindIndex = false;
    if(fileinfo.isFile() && config->isSupportedImageFormat(path)) {
        //if(fileinfo.isFile() && config->isSupportedImageFormat(fileinfo.completeSuffix())) {
        path = fileinfo.dir().absolutePath();
        willFindIndex = true;
    }
    if(m_root_volume) {
        saveState();
    }
    std::shared_ptr< MangaVolume> volume = openVolume(path);
    if (volume == nullptr) {
        qWarning() << "Could not open path" << filepath;
        return;
    }
    m_root_volume.reset();
    if(volume->numPages() == 0) {
        return;
    }
    m_root_volume = volume;
    m_filename = fileinfo.fileName();//Now that we know taht we've opened the root vol set the filename...
    if(m_root_volume) {
        m_root_volume->getNumRenderWidgets(m_renderwidgets.size());
        m_curindex = m_fsmodel.index(path);
    }
    m_page_num = -1;
    emit newRootMangaVolume(m_root_volume);
    loadState();
    qWarning() << "Loading state...";
    if(willFindIndex && m_root_volume) {
        qWarning() << "Finding index: " << m_root_volume->findIndex(path);
        emit changePage(m_root_volume->findIndex(filepath));
    }
    centralWidget()->setFocus();
}

std::shared_ptr<MangaVolume> MainWindow::openVolume(const QString & filename) {
    MangaVolume *volume = MangaVolume::createVolume(filename);

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
    if (index > m_root_volume->size()-m_renderwidgets.size()) {
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
#ifdef USE_NETWORKING
    m_remote_client.manager().set_page(m_page_num);
#else
    m_state_mgr.set_page(m_page_num);
#endif
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
    int keyvalue = event->key() | event->modifiers();
    auto&& kmfit = m_keys.find(keyvalue);
    if(kmfit != m_keys.end())  {
        (*kmfit->second)();
    } else {
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
    qWarning() << "Root path changed: " << m_fsmodel.rootPath();
    emit emitRootPath(dir.absolutePath());

}

void MainWindow::changeVolume(int index){
    qWarning() << "Changing volume" << index;
    QModelIndex newind = m_fsmodel.index(m_curindex.row()+index,m_curindex.column(),m_curindex.parent());
    qWarning() << "Current: " << m_fsmodel.filePath(m_curindex);
    qWarning() << "Parent: " << m_fsmodel.filePath(m_curindex.parent());
    qWarning() << "Next: " << m_fsmodel.filePath(newind);
    qWarning() << "Root Path: " << m_fsmodel.rootPath();
    if(newind.isValid())
    {

        openRootVolume(m_fsmodel.filePath(newind));
        if(m_root_volume && index < 0) {
            changePage(m_root_volume->size() - m_renderwidgets.size());
        }
    }
}
void MainWindow::loadState() {
#ifdef USE_NETWORKING
    const SaveState& state = m_remote_client.sync(m_filename.toUtf8().data());
#else
    const SaveState& state = m_state_mgr.get_state(m_filename.toUtf8().data());
#endif
    changePage(state.currentpage());
}

void MainWindow::saveState() {
    qWarning() << "Saving state... ";
    if(m_root_volume->numPages()-m_renderwidgets.size() ==  m_page_num) {
#ifdef USE_NETWORKING
        m_remote_client.manager().set_page(0);
#else
        m_state_mgr.set_page(0);
#endif
    }
#ifdef USE_NETWORKING
    m_remote_client.manager().save_state();
    m_remote_client.sync();
#else
    m_state_mgr.save_state();
#endif
}
#include <iostream>
MainWindow::~MainWindow() {
    std::cout << "Mainwindow being destoryed.." << std::endl;
    if(m_root_volume) {
        qWarning() << "root volume existed";
        saveState();
    } else {
        qWarning() << "No root volume..";
    }
}
