#include "include/mangavolume.h"
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QCryptographicHash>
#include <QTime>
#include <QDebug>
#include <QDir>



CompressedFileMangaVolume::CompressedFileMangaVolume(const QString filepath, QObject *parent)
    : MangaVolume(true,parent) {
    QStringList path_split = filepath.split("/");
    QString filename = path_split.last();
    QStringList filename_split = filename.split(".");
    if (filename_split.length() > 1) {
        filename_split.pop_back();
    }


    QDir dir;
    do {
        // keep trying hashes until dir exists.
        // no one could have taken all hashes
        QTime time = QTime::currentTime();
        QString toHash = filepath+time.toString();
        qWarning() <<  toHash;
        QString hash = QString(QCryptographicHash::hash(
                                   toHash.toAscii(),
                                   QCryptographicHash::Sha1).toHex());
        m_file_dir = tr("/tmp/") + filename_split.join(tr("."))
                + tr("-") + hash;
        qWarning() << "Making directory: " << m_file_dir;
        dir = QDir(m_file_dir);
    } while (dir.exists());
    dir.mkpath(".");





    QString program = "";
    QStringList arguments;
    if (filename.endsWith(tr(".zip"))) {
        program = tr("/usr/bin/unzip");
        arguments << tr("-d") << m_file_dir;
        arguments << filepath;
    } else if (filename.endsWith(tr(".rar"))) {
        program = tr("/usr/bin/unrar");
        arguments << tr("x");
        arguments << filepath;
        arguments << m_file_dir;
    }

    qWarning() << "Open file?: " << filename;
    QProcess * myProcess = new QProcess(this);
    myProcess->start(program, arguments);
    myProcess->waitForFinished();
    if (myProcess->exitCode() ==0) {
        qWarning() << "Extracted successfully";
        m_do_cleanup = true;
        readImages(m_file_dir);
        for (const MangaPage& page: m_pages) {
            page.getFilename().size();
            // TODO(mtao): processing?
        }
    }
}


std::shared_ptr<const QImage> CompressedFileMangaVolume::getImage(uint page_num, QPointF) const {
    if (page_num >= m_pages.size()) {
        return std::shared_ptr<const QImage>();
    } else {
        return std::shared_ptr<const QImage>(&m_pages.at(page_num).getData());
    }
}


void CompressedFileMangaVolume::readImages(const QString & path) {
    QFileInfo fileInfo(path);
    if (fileInfo.isDir()) {
        QDir dir(path);
        QStringList fileList = dir.entryList(
                    QDir::AllEntries | QDir::NoDotAndDotDot
                    | QDir::NoSymLinks | QDir::Hidden, QDir::Name
                    | QDir::IgnoreCase);
        for (int i = 0; i < fileList.count(); ++i) {
            readImages(path + tr("/")+fileList.at(i));
        }
    } else {
        MangaPage img(path);
        if (!img.isNull())
            m_pages.push_back(img);
    }
}


void CompressedFileMangaVolume::cleanUp(const QString &path) {
    QFileInfo fileInfo(path);
    if (fileInfo.isDir()) {
        QDir dir(path);
        QStringList fileList = dir.entryList(
                    QDir::AllEntries | QDir::NoDotAndDotDot
                    | QDir::NoSymLinks | QDir::Hidden, QDir::Name
                    | QDir::IgnoreCase);
        for (int i = 0; i < fileList.count(); ++i) {
            cleanUp(path + tr("/")+fileList.at(i));
        }
        dir.rmpath(tr("."));
    } else {
        QFile::remove(path);
    }
}



PDFMangaVolume::PDFMangaVolume(const QString filepath, QObject *parent): MangaVolume(false, parent){
    m_doc.reset(Poppler::Document::load(filepath));
    m_doc->setRenderHint(Poppler::Document::Antialiasing);
    m_doc->setRenderHint(Poppler::Document::TextAntialiasing);
}

std::shared_ptr<const QImage> PDFMangaVolume::getImage(uint page_num, QPointF scale) const {
    if(scale.x() != scale.x()) {
        scale.setX(1.0f);
    }
    if(scale.y() != scale.y()) {
        scale.setY(1.0f);
    }

    std::shared_ptr<const QImage> img;
    qWarning() << scale;
    if(m_doc && page_num < m_doc->numPages()) {
        img = std::shared_ptr<const QImage>(
                    new const QImage(m_doc->page(page_num)->renderToImage(72.0f*scale.x(),72.0f*scale.y()))
                        );
                //m_active_pages.insert(img);
    } else {
        return std::shared_ptr<const QImage>();
    }/*
    for (auto it = m_active_pages.begin(); it != m_active_pages.end();) {
        if(!*it)
        {
            it = m_active_pages.erase(it);
        } else {
            ++it;
        }
    }*/
    return img;
}
