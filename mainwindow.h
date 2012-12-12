#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>
#include <memory>
#include "mangavolume.h"
class RenderWidget;
class QDockWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    private:
    QDockWidget * m_control;
    uint m_num_renderers;
    uint m_index;
    std::unique_ptr<MangaVolume> m_volume;

    private:
    RenderWidget * createRenderWidget();
    public slots:
    void openFile();
    void openFile(const QString & filepath);
    void nextPage();
    void previousPage();
    void changePage(uint i);

    signals:

    void pageChanged();
};

#endif // MAINWINDOW_H
