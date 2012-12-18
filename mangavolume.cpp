#include "mangavolume.h"
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QCryptographicHash>
#include <QTime>
#include <QDebug>
#include <QDir>

const QImage MangaVolume::m_null_image = QImage();
MangaVolume::MangaVolume(const QString filepath, QObject *parent)
    : QObject(parent) , m_do_cleanup(false) {
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

const QImage & MangaVolume::getImage(uint page_num) const {
    if (page_num >= m_pages.size()) {
        return m_null_image;
    } else {
        return m_pages.at(page_num).getData();
    }
}


void MangaVolume::readImages(const QString & path) {
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


void MangaVolume::cleanUp(const QString &path) {
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


MangaVolume::~MangaVolume() {
    if (m_do_cleanup) {
        cleanUp(m_file_dir);
    }
}
