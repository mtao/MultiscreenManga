#ifndef MANGAVOLUME_H
#define MANGAVOLUME_H

#include "include/configuration.h"
#include <QObject>
#include <vector>
#include <QImage>
#include <QDir>
#include <poppler/qt4/poppler-qt4.h>
#include <memory>
#include <set>


class MangaPage
{
public:
    MangaPage() {}
    MangaPage(const QString & path): filepath(path)
      , filename(filepath.split("/").last())
      ,data(new QImage(path))
    {}


    bool isNull()const {if(!data) return true; return data->isNull();}
    const QString & getFilepath()const {return filepath;}
    const QString & getFilename()const {return filename;}
    std::shared_ptr<const QImage> getData()const {return data;}

private:
    QString filepath;
    QString filename;
    std::shared_ptr<QImage> data;
};


class MangaVolume : public QObject
{
    Q_OBJECT
public:
    explicit MangaVolume(bool do_cleanup, QObject * parent = 0): QObject(parent) {}
    // Number of pages in this volume
    virtual uint numPages() const {return size();};
    // Number of pages in this volume and all subvolumes
    virtual uint size() const = 0;
    virtual std::shared_ptr<const QImage> getImage (uint page_num, QPointF scale=QPointF(1.0f,1.0f)) = 0;
    virtual void discardPage(uint page_num) {}
    virtual bool refreshOnResize() const {return false;}
    virtual int findIndex(const QString &) { return -1; }
    static MangaVolume * createVolume(const QString & filepath);
public slots:
    virtual void getNumRenderWidgets(int) {}
};

class DirectoryMangaVolume : public MangaVolume
{
public:
    explicit DirectoryMangaVolume(QObject * parent = 0, int prefetch_width=-1);
    explicit DirectoryMangaVolume(const QString & dirpath, QObject *parent = 0, int prefetch_width=-1);
    std::shared_ptr<const QImage> getImage(uint page_num, QPointF);
    virtual void discardPage(uint page_num);
    // Number of pages in this volume
    virtual uint numPages() const;
    // Number of pages in this volume and all subvolumes
    virtual uint size() const;
    virtual int findIndex(const QString & filename);


protected:
    QString m_file_dir;
    std::vector<MangaPage> m_pages;
    std::vector<QString> m_page_names;
    std::set<uint> m_active_pages;
    std::map<uint, MangaPage> m_prefetched_pages;
    int m_prefetch_width;
    int m_num_renderwidgets;

    virtual void readImages(const QString & path);
    virtual void getNumRenderWidgets(int);


private:
    int prefetchMin();
    int prefetchMax();
    void prefetch();
    QList<QString> m_child_volumes;
};

/**
 * CFMV subclasses DMV, since it's current behavior is to extract the archive,
 * then attempt to treat it like a DMV.
 */
class CompressedFileMangaVolume : public DirectoryMangaVolume
{
public:
    ~CompressedFileMangaVolume();

protected:
    //protected constructor because this doesn't know what program or arguments to use
    explicit CompressedFileMangaVolume(const QString & filepath, QObject *parent = 0, bool do_cleanup = true);
    void extractToDir();
    void createOutputDir(const QString & filepath);
    
    void cleanUp(const QString & path);
    QString m_programName;
    QStringList m_programArguments;
    bool m_do_cleanup;
};

class ZipFileMangaVolume: public CompressedFileMangaVolume {
    public:
    explicit ZipFileMangaVolume(const QString & filepath
            , QObject *parent = 0, bool do_cleanup = true);
};
class RarFileMangaVolume: public CompressedFileMangaVolume {
    public:
    explicit RarFileMangaVolume(const QString & filepath
            , QObject *parent = 0, bool do_cleanup = true);
};


class PDFMangaVolume : public MangaVolume
{
public:
    explicit PDFMangaVolume(const QString filepath, QObject *parent = 0);
    uint size() const;
    std::shared_ptr<const QImage> getImage (uint page_num, QPointF scale);
    bool refreshOnResize() const {return true;}

private:
    std::unique_ptr<Poppler::Document> m_doc;
    std::set<std::shared_ptr<const QImage> > m_active_pages;
    QString m_file_dir;

    void readImages(const QString & path);
};

#endif // MANGAVOLUME_H
