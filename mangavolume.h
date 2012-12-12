#ifndef MANGAVOLUME_H
#define MANGAVOLUME_H

#include <QObject>
#include <vector>
#include <QImage>
#include <QDir>

struct MangaPage
{
    MangaPage(const QString & path): filepath(path)
      , filename(filepath.split("/").last())
      ,data(QImage(path))
    {}

bool isNull()const {return data.isNull();}

const QString filepath;
const QString filename;
const QImage data;
};

class MangaVolume : public QObject
{
    Q_OBJECT
public:
    explicit MangaVolume(const QString filepath, QObject *parent = 0);
    ~MangaVolume();
    uint size() const {return m_pages.size();}
    
signals:

public slots:

private:
    QString m_file_dir;
    std::vector<MangaPage> m_pages;

    void cleanUp(const QString & path);
    void readImages(const QString & path);
    bool m_do_cleanup = false;
};

#endif // MANGAVOLUME_H
