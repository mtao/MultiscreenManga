#ifndef SIDEBAR_H
#define SIDEBAR_H
#include <QWidget>
#include <QTreeView>
class QFileSystemModel;

class FileViewer: public QTreeView
{
    Q_OBJECT
public:
    FileViewer(QFileSystemModel * model, QWidget *parent = 0);
protected:
    void mousePressEvent(QMouseEvent * event);
public slots:
    void selectRoot();
    void setRoot(const QString & dirname);
};

class Sidebar: public QWidget
{
    Q_OBJECT

public:
    Sidebar(QFileSystemModel * model,QWidget *parent = 0);
public slots:
    void modelItemSelected(const QModelIndex & modelIndex);
signals:
    void filePathSelected(const QString &);
private:
    FileViewer * tree;
};
#endif
