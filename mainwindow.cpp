#include "mainwindow.h"
#include "renderwidget.h"
#include <QtGui/QDockWidget>
#include <QtGui/QMenuBar>
#include <QtGui/QFileDialog>
#include "mangavolume.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{

    setMenuBar(new QMenuBar(this));
    QMenu *fileMenu = menuBar() -> addMenu(tr("&File"));
    QAction *openAct = new QAction( tr("&Open"), this );
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(openFile()));
    //Quit action
    QAction *quitAct = new QAction( tr("&Quit"), this );
    quitAct->setShortcuts(QKeySequence::Quit);
    quitAct->setStatusTip(tr("Quit"));
    connect(quitAct, SIGNAL(triggered()), this, SLOT(close()));

    fileMenu->addAction(openAct);
    fileMenu->addAction(quitAct);


    m_control =new QDockWidget(tr("Control"),this);
    addDockWidget(Qt::LeftDockWidgetArea, m_control);


    RenderWidget * main_widget = createRenderWidget();

    setCentralWidget(main_widget);

}

RenderWidget * MainWindow::createRenderWidget()
{
    RenderWidget * widget = new RenderWidget(this);

    return widget;
}

void MainWindow::openFile()
{
    openFile(QFileDialog::getOpenFileName(0, tr("Choose file"), QString()));
}

void MainWindow::openFile(const QString & filepath)
{
    m_volume.reset(new MangaVolume(filepath, this));
}

void MainWindow::nextPage()
{
    changePage(m_index+m_num_renderers);
}
void MainWindow::previousPage()
{
    changePage(m_index-m_num_renderers);
}

void MainWindow::changePage(uint i)
{
    if(!m_volume) return;
    if(i != m_index)
    {
        m_index = i;
        if(m_index >= m_volume->size())
        {
            m_index = m_volume->size()-1-m_num_renderers;
        }

        emit pageChanged();
    }
}

MainWindow::~MainWindow()
{

}
