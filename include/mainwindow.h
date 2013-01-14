#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>
#include <memory>
#include <mutex>
#include <set>
#include <stack>
#include "mangavolume.h"
#include "renderwidget.h"
class QDockWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    QDockWidget * m_control;
    int m_page_num;
    std::shared_ptr<MangaVolume> m_root_volume;
    std::mutex m_renderer_mutex;

    std::shared_ptr<MangaVolume> openVolume(const QString & filename);
    //Don't need need to be smart because I'm taking care of this myself
    std::set< std::unique_ptr<RenderWidgetResource> > m_renderwidgets;
    std::shared_ptr<Configuration> config;

protected:
    void setFocus();

public slots:
    void openRootVolume();
    void openRootVolume(const QString & filepath, bool changeRoot = false);
    void nextPage();
    void previousPage();
    void changePage(int index);
    void renderWidgetClosed(uint index);
    RenderWidget * createRenderWidget();
    void closeEvent(QCloseEvent *event);
    void keyPressEvent(QKeyEvent *);
    void setRoot(const QString & rootpath);


signals:
    void newRootMangaVolume(std::shared_ptr<MangaVolume> volume);
    void newPage(uint page);
    void wantNewPage();
    void wantPrevPage();
    void closeAll();
    void numRenderWidgets(int);
    void emitRootPath(const QString &);

    void pageChanged();
};

#endif // MAINWINDOW_H
