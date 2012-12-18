#ifndef MANGAVOLUME_H
#define MANGAVOLUME_H

#include <QObject>
#include <vector>
#include <QImage>
#include <QDir>

class MangaPage
{
public:
    MangaPage(const QString & path): filepath(path)
      , filename(filepath.split("/").last())
      ,data(QImage(path))
    {}

    bool isNull()const {return data.isNull();}
    const QString & getFilepath()const {return filepath;}
    const QString & getFilename()const {return filename;}
    const QImage & getData()const {return data;}


private:
    QString filepath;
    QString filename;
    QImage data;
};

class MangaVolume : public QObject
{
    Q_OBJECT
public:
    explicit MangaVolume(const QString filepath, QObject *parent = 0);
    ~MangaVolume();
    uint size() const {return m_pages.size();}
    const QImage & getImage (uint page_num) const;
    
signals:

public slots:

private:
    QString m_file_dir;
    const static QImage m_null_image;
    std::vector<MangaPage> m_pages;

    void cleanUp(const QString & path);
    void readImages(const QString & path);
    bool m_do_cleanup;
};

#endif // MANGAVOLUME_H
