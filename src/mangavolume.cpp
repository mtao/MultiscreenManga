#include "include/mangavolume.h"
#include "include/configuration.h"
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QCryptographicHash>
#include <QTime>
#include <QDebug>
#include <QDir>


MangaVolume * MangaVolume::createVolume(const QString &filepath) {
    Configuration::FileType format = Configuration().getVolumeFormat(filepath);
    switch(format) {
    case Configuration::PDF:
        return new PDFMangaVolume(filepath);
    case Configuration::DIRECTORY:
        return new DirectoryMangaVolume(filepath);
    case Configuration::ZIP:
        return new ZipFileMangaVolume(filepath);
    case Configuration::RAR:
        return new RarFileMangaVolume(filepath);
        case Configuration::IMAGE:
        return new DirectoryMangaVolume(QFileInfo(filepath).absolutePath());
        default:
        return nullptr;
    }
}


// DirectoryMangaVolume

DirectoryMangaVolume::DirectoryMangaVolume(QObject * parent, int prefetch_width)
    : MangaVolume(parent), m_prefetch_width(prefetch_width)
{}

DirectoryMangaVolume::DirectoryMangaVolume(const QString & dirpath, QObject *parent, int prefetch_width)
    : MangaVolume(false, parent), m_prefetch_width(prefetch_width), m_num_renderwidgets(1) {
    readImages(dirpath);
}

uint DirectoryMangaVolume::size() const {
    return m_page_names.size();
}

uint DirectoryMangaVolume::numPages() const {
    return size();
}

int DirectoryMangaVolume::findIndex(const QString & filename) {
    QString filepath_canonical = QFileInfo(filename).canonicalFilePath();
    int ind = 0;
    for(auto&& it=m_page_names.cbegin(); it != m_page_names.cend(); ++it, ++ind) {
        if (QFileInfo(*it).canonicalFilePath() == filepath_canonical) {
            return ind;
        }
    }
    return -1;
}

std::shared_ptr<const QImage> DirectoryMangaVolume::getImage(uint page_num, QPointF) {
    if (page_num >= m_page_names.size()) {
        return std::shared_ptr<const QImage>();
    } else if(m_prefetch_width <= 0) {
        return MangaPage(m_page_names[page_num]).getData();
    } else {
        m_active_pages.insert(page_num);
        prefetch();
        return m_prefetched_pages[page_num].getData();

    }


}

void DirectoryMangaVolume::getNumRenderWidgets(int count) {
    m_num_renderwidgets = count;
    prefetch();


}

void DirectoryMangaVolume::readImages(const QString & path) {
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
        QString extension = path.split(".").last();
        auto formats = Configuration().getSupportedImageFormats();
        if (Configuration().isSupportedImageFormat(fileInfo.suffix())) {
            m_page_names.push_back(fileInfo.canonicalFilePath());
            /*
            MangaPage img(path);
            if (!img.isNull()) {
                m_pages.push_back(img);
            }*/
        } else {
            qDebug() << "Skipping file with unknown extension " << path;
        }
    }
}

int DirectoryMangaVolume::prefetchMin() {
    if(m_active_pages.size() == 0) return 0;
    return std::max(0
                    ,static_cast<int>(*m_active_pages.cbegin())-m_prefetch_width*m_num_renderwidgets);
}

int DirectoryMangaVolume::prefetchMax() {
    if(m_active_pages.size() == 0) return 0;
    return std::min((int)m_page_names.size()
                    ,static_cast<int>(*m_active_pages.crbegin())+m_prefetch_width*m_num_renderwidgets);
}

void DirectoryMangaVolume::discardPage(uint page_num) {
    if(m_prefetch_width <= 0) return;
    //qWarning() << "Discard!" << page_num;
    //qWarning() << "Old size: " << m_active_pages.size();
    m_active_pages.erase(page_num);
    //qWarning() << "New size: "<<m_active_pages.size();
}
void DirectoryMangaVolume::prefetch() {
    int theoretical_min = prefetchMin();
    int theoretical_max = prefetchMax();
    theoretical_max = std::max(theoretical_min+1,theoretical_max);
    //qWarning() << theoretical_min << " " << theoretical_max;
    //clear out pages that aren't necessary anymore
    while(m_prefetched_pages.size() > 0 &&
          m_prefetched_pages.cbegin()->first < theoretical_min) {
        //qWarning() << "front unprefetched: " << m_prefetched_pages.begin()->first;
        m_prefetched_pages.erase(m_prefetched_pages.begin());
    }
    while(m_prefetched_pages.size() > 0 &&
          m_prefetched_pages.crbegin()->first > theoretical_max) {
        //qWarning() << "back unprefetched: " << m_prefetched_pages.crbegin()->first;
        //qWarning() << m_prefetched_pages.crbegin()->second.getFilename();
        m_prefetched_pages.erase(--(m_prefetched_pages.crbegin().base()));
    }
    //if empty fill it in
    if( m_prefetched_pages.size() == 0) {
        for(int i=theoretical_min; i < theoretical_max; ++i) {
            m_prefetched_pages[i] = MangaPage(m_page_names[i]);
        }
    }
    //else fill in the front and back
    if(theoretical_min < m_prefetched_pages.cbegin()->first) {
        int endPos = m_prefetched_pages.cbegin()->first;
        for(int i=theoretical_min; i < endPos; ++i) {
            //qWarning() << "font prefetched: " << i;
            m_prefetched_pages[i] = MangaPage(m_page_names[i]);
        }
    }
    if(theoretical_max > m_prefetched_pages.crbegin()->first) {
        int startPos = m_prefetched_pages.crbegin()->first+1;
        for(int i=startPos; i < theoretical_max; ++i) {
            //qWarning() << "back prefetched: " << i;
            m_prefetched_pages[i] = MangaPage(m_page_names[i]);
        }
    }

}

// CompressedFileMangaVolume

CompressedFileMangaVolume::CompressedFileMangaVolume(const QString & filepath, QObject *parent, bool do_cleanup)
    : DirectoryMangaVolume(parent), m_do_cleanup(do_cleanup) {
    createOutputDir(filepath);//Sets m_file_dir
}

void CompressedFileMangaVolume::createOutputDir(const QString & filepath) {
    QStringList path_split = filepath.split("/");
    QString filename = path_split.last();
    QStringList filename_split = filename.split(".");
    if (filename_split.length() > 1) {
        filename_split.pop_back();//for whatever reason we don't want the last file extension
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
}

void CompressedFileMangaVolume::extractToDir() {
//    QString program = "";
//    QStringList arguments;
//    setProgramAndArguments(filepath,m_file_dir,program,arguments);


    QProcess * myProcess = new QProcess(this);

    // Start the extraction program
    myProcess->start(m_programName, m_programArguments);

    // Check to make sure it started correctly
    if (!myProcess->waitForStarted()) {
        switch (myProcess->error()) {
        case QProcess::FailedToStart:
            qWarning() << "Failed to start program" << m_programName<< ". Is it installed correctly?";
            break;
        case QProcess::Crashed:
            qWarning() << "Program" << m_programName << "crashed.";
            break;
        default:
            qWarning() << "QProcess::ProcessError code " << myProcess->error();
        }
        return;
    }

    // Check to make sure it finished correctly
    if (!myProcess->waitForFinished()) {
        qWarning() << m_programName << "was unable to complete with arguments" << m_programArguments;
        // TODO(umbrant): capture stdout/stderr to show the user
        return;
    }

    // Successful extraction
    qWarning() << "Extracted successfully";
    m_do_cleanup = true;
    readImages(m_file_dir);
    for (const MangaPage& page: m_pages) {
        page.getFilename().size();
        // TODO(mtao): processing?
    }

}

CompressedFileMangaVolume::~CompressedFileMangaVolume() {
    if (m_do_cleanup) {

        cleanUp(m_file_dir);
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

//ZipFileMangaVolume

ZipFileMangaVolume::ZipFileMangaVolume(const QString & filepath, QObject *parent, bool do_cleanup )
    : CompressedFileMangaVolume(filepath
                                ,parent,do_cleanup) {
    m_programName = tr("unzip");
    m_programArguments.clear();
    m_programArguments << tr("-d") << m_file_dir;
    m_programArguments << filepath;
    extractToDir();

}


//RarFileMangaVolume
RarFileMangaVolume::RarFileMangaVolume(const QString & filepath, QObject *parent, bool do_cleanup )
    : CompressedFileMangaVolume(filepath
                                ,parent,do_cleanup)
{
    m_programName= tr("unrar");
    m_programArguments << tr("x");
    m_programArguments << filepath;
    m_programArguments << m_file_dir;
    extractToDir();
}


// PDFMangaVolume

PDFMangaVolume::PDFMangaVolume(const QString filepath, QObject *parent): MangaVolume(false, parent){
    m_doc.reset(Poppler::Document::load(filepath));
    m_doc->setRenderHint(Poppler::Document::Antialiasing);
    m_doc->setRenderHint(Poppler::Document::TextAntialiasing);
}

std::shared_ptr<const QImage> PDFMangaVolume::getImage(uint page_num, QPointF scale) {
    if(scale.x() != scale.x()) {
        scale.setX(1.0f);
    }
    if(scale.y() != scale.y()) {
        scale.setY(1.0f);
    }

    std::shared_ptr<const QImage> img;
    //qWarning() << scale;
    if(m_doc && page_num < m_doc->numPages()) {
        img = std::shared_ptr<const QImage>(
                    new const QImage(m_doc->page(page_num)->renderToImage(120.0f*scale.x(),120.0f*scale.y()))
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

uint PDFMangaVolume::size() const {
    if ( m_doc ) {
        return m_doc->numPages();
    } else {
        return 0;
    }
}
