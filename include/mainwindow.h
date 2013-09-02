#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>
#include <vector>
#include <memory>
#include <mutex>
#include <set>
#include <stack>
#include "mangavolume.h"
#include "renderwidget.h"
#include "savestate.h"
#include "remotesyncclient.h"
class QDockWidget;

struct KeyMappableFunction {
    QString short_name;
    QString name;
    std::function<void(void)> func;
    void operator()(){func();}
    typedef std::shared_ptr<KeyMappableFunction> ptr;
    //Is there no better C++11 way to do this? i want moves but to allow for other permutations a lot more effort is needed.  good thing this is just for intiializing the key bindings
    KeyMappableFunction(QString&& short_name, QString && name, std::function<void(void)>&& func)
        : short_name(short_name), name(name), func(func) {}
    KeyMappableFunction(const QString& short_name, const QString & name, const std::function<void(void)>& func)
        : short_name(short_name), name(name), func(func) {}
    KeyMappableFunction();
};

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
    std::map< int, KeyMappableFunction::ptr > m_keys;
    std::shared_ptr<Configuration> config;
    QString m_filename;
    QDir m_root_dir;
    QFileSystemModel m_fsmodel;
    QModelIndex m_curindex;

protected:
    void setFocus();
    void mousePressEvent(QMouseEvent *);
    void wheelEvent(QWheelEvent *);

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
    void setRoot(const QString & dirpath);
    void setRoot(const QDir & dir);
    void changeVolume(int index);


signals:
    void newRootMangaVolume(std::shared_ptr<MangaVolume> volume);
    void newPage(uint page);
    void wantNewPage();
    void wantPrevPage();
    void closeAll();
    void numRenderWidgets(int);
    void emitRootPath(const QString &);

    void pageChanged();
private:
    void toggleBars(bool show);
    bool showBars=true;//by default show bars
    void initializeKeyBindings();
    void saveState();
    void loadState();
#ifdef USE_NETWORKING
    RemoteSyncClient m_remote_client;
#else
    SaveStateManager m_state_mgr;
#endif

    KeyMappableFunction::ptr& addKeyMappableFunction(KeyMappableFunction::ptr&& ptr);
    KeyMappableFunction::ptr& addKeyMappableFunction(QString&& short_name, QString && name,std::function<void(void)>&&);
    std::map<QString,KeyMappableFunction::ptr> m_available_keys;
};

#endif // MAINWINDOW_H
