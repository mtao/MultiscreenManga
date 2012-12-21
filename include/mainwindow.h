#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>
#include <memory>
#include <mutex>
#include <set>
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
    uint m_page_num;
    std::shared_ptr<const MangaVolume> m_volume;
    std::mutex m_renderer_mutex;

    private:
    //Don't need need to be smart because I'm taking care of this myself
    std::set< std::unique_ptr<RenderWidgetResource> > m_renderwidgets;

    protected:
    void setFocus();
    public slots:
    void openFile();
    void openFile(const QString & filepath);
    void nextPage();
    void previousPage();
    void changePage(uint index);
    void renderWidgetClosed(uint index);
    RenderWidget * createRenderWidget();
    void closeEvent(QCloseEvent *event);
    void keyPressEvent(QKeyEvent *);


    signals:
    void newMangaVolume(std::shared_ptr<const MangaVolume> volume);
    void newPage(uint page);
    void wantNewPage();
    void wantPrevPage();
    void closeAll();

    void pageChanged();
};

#endif // MAINWINDOW_H
