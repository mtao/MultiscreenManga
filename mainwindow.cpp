#include "mainwindow.h"
#include <QtGui/QDockWidget>
#include <QtGui/QMenuBar>
#include <QtGui/QFileDialog>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{

    setMenuBar(new QMenuBar(this));
    QMenu *fileMenu = menuBar() -> addMenu(tr("&File"));
    QAction *openAct = new QAction( tr("&Open"), this );
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(openFile()));

    QAction *createRenderWidgetAct = new QAction( tr("&Render Window"), this );
//    createRenderWidgetAct->setShortcuts(QKeySequence::Open);
    createRenderWidgetAct->setStatusTip(tr("New Render Window"));
    connect(createRenderWidgetAct, SIGNAL(triggered()), this, SLOT(createRenderWidget()));
    //Quit action
    QAction *quitAct = new QAction( tr("&Quit"), this );
    quitAct->setShortcuts(QKeySequence::Quit);
    quitAct->setStatusTip(tr("Quit"));
    connect(quitAct, SIGNAL(triggered()), this, SLOT(close()));

    fileMenu->addAction(openAct);
    fileMenu->addAction(createRenderWidgetAct);
    fileMenu->addAction(quitAct);


    m_control =new QDockWidget(tr("Control"),this);
    addDockWidget(Qt::LeftDockWidgetArea, m_control);


    RenderWidget * main_widget = createRenderWidget();
    main_widget->setParent(this);
//    main_widget->setMain();

    setCentralWidget(main_widget);

}

RenderWidget * MainWindow::createRenderWidget()
{
    m_renderer_mutex.lock();
    qWarning() << "New window: " << m_page_num << " " << m_renderwidgets.size();
    RenderWidget * widget = new RenderWidget(
                m_renderer_mutex
                , m_page_num
                , m_renderwidgets.size()
                , m_volume
                );
    connect(this, SIGNAL(newMangaVolume(std::shared_ptr<const MangaVolume>)),
            widget, SLOT(setMangaVolume(std::shared_ptr<const MangaVolume>)));
    connect(this, SIGNAL(newPage(uint)),
            widget, SLOT(setPage(uint)));
    connect(this, SIGNAL(closeAll()),
            widget, SLOT(close()));
    connect(widget, SIGNAL(renderWidgetClosing(uint)), this, SLOT(renderWidgetClosed(uint)));
    widget->show();
    m_renderwidgets.insert(std::unique_ptr<RenderWidgetResource>(new RenderWidgetResource(widget,m_renderwidgets.size())));
    m_renderer_mutex.unlock();

    return widget;
}
void MainWindow::closeEvent(QCloseEvent *event)
{
    emit closeAll();
    QMainWindow::closeEvent(event);
}

void MainWindow::openFile()
{
    openFile(QFileDialog::getOpenFileName(0, tr("Choose file"), QString()));
}

void MainWindow::openFile(const QString & filepath)
{
    m_volume.reset(new MangaVolume(filepath, this));
    m_page_num = 0;
    emit newMangaVolume(m_volume);
}

void MainWindow::nextPage()
{
    changePage(m_page_num+m_renderwidgets.size());
}
void MainWindow::previousPage()
{
    changePage(m_page_num-m_renderwidgets.size());
}

void MainWindow::changePage(uint index)
{
    m_page_num = index;
    if(m_page_num >= m_volume->size())
    {
        m_page_num = m_volume->size()-1-m_renderwidgets.size();
    }
    emit newPage(m_page_num);
}
void MainWindow::renderWidgetClosed(uint index)
{
    bool need_to_unlock_myself=false;
    if(m_renderer_mutex.try_lock())
    {
        qWarning() << "Someone didn't lock mutex before starting index decrementing!  I've locked it for myself now";
        need_to_unlock_myself = true;
    }

    auto it = m_renderwidgets.begin();
    int count=0;
    for(;(*it)->index != index; ++count,++it);
    it = m_renderwidgets.erase(it);
    for(;it != m_renderwidgets.end(); ++it)
    {
        (*it)->decrementIndex();
    }


    if(need_to_unlock_myself)
    {
        m_renderer_mutex.unlock();
    }
}


MainWindow::~MainWindow()
{

}