#include "include/configuration.h"
#include <magic.h>
#include <QImageReader>
// ConfigurationHidden

ConfigurationHidden::ConfigurationHidden()
{
    // Read in Qt's reported supported image formats
    QList<QByteArray> formatsBytes = QImageReader::supportedImageFormats();
    for (auto && format: formatsBytes) {
        supportedImageFormats.push_back(QString(format));
    }
    // Set our supported volume formats
    // TODO(umbrant): try to avoid hardcoding this
    supportedVolumeFormats.push_back(QString("zip"));
    supportedVolumeFormats.push_back(QString("rar"));
    supportedVolumeFormats.push_back(QString("cbz"));
    supportedVolumeFormats.push_back(QString("cbr"));
    supportedVolumeFormats.push_back(QString("pdf"));

    // Construct supported file filters based on
    // supported image and volume formats
    supportedFileFilters.append("Supported image and archive formats (");
    for (auto && img_format: supportedImageFormats) {
        supportedFileFiltersList.push_back("*."+img_format);
    }
    for (auto && vol_format: supportedVolumeFormats) {
        supportedFileFiltersList.push_back("*."+vol_format);
    }
    for (auto && format: supportedFileFiltersList) {
        supportedFileFilters.append(format+" ");
    }

    supportedFileFilters.append(");; All files (*.*)");
}

ConfigurationHidden & ConfigurationHidden::getInstance() {
    static ConfigurationHidden * ch = new ConfigurationHidden();
    return *ch;
}

// Configuration

Configuration::Configuration(): config(ConfigurationHidden::getInstance()) {}

const QStringList & Configuration::getSupportedImageFormats() const {
    return config.supportedImageFormats;
}

const QStringList & Configuration::getSupportedVolumeFormats() const {
    return config.supportedVolumeFormats;
}

const QStringList & Configuration::getSupportedFileFiltersList() const {
    return config.supportedFileFiltersList;
}

const QString & Configuration::getSupportedFileFilters() const {
    return config.supportedFileFilters;
}

const bool Configuration::isSupportedImageFormat(const QString & str) const {
    return getSupportedImageFormats().contains(str.toLower());
}
const bool Configuration::isSupportedVolumeFormat(const QString & str) const {
    return getSupportedVolumeFormats().contains(str.toLower());
}


//This is borrowed from 
//http://va-sorokin.blogspot.ca/2011/03/how-to-get-mime-type-on-nix-system.html
#include <QDebug>
QString Configuration::getMimeType(const QFileInfo& info) {
    QString result("application/octet-stream");
    magic_t magicMimePredictor;
    magicMimePredictor = magic_open(MAGIC_MIME_TYPE); // Open predictor
    if (!magicMimePredictor) {
        qDebug() << "libmagic: Unable to initialize magic library";
    }
    else
    {
        if (magic_load(magicMimePredictor, 0)) { // if not 0 - error
            qDebug() << "libmagic: Can't load magic database - " +
                        QString(magic_error(magicMimePredictor));
            magic_close(magicMimePredictor); // Close predictor
        }
        else
        {
            char *file = info.canonicalFilePath().toAscii().data();
            const char *mime;
            mime = magic_file(magicMimePredictor, file); // getting mime-type
            result = QString(mime);
            magic_close(magicMimePredictor); // Close predictor
        }
    }
    qDebug() << "libmagic: result mime type - " + result + "for file: " +
                info.canonicalFilePath();
    return result;
}

Configuration::FileType Configuration::getVolumeFormat(const QString & filepath) const {
    QFileInfo info(filepath);
    if(!info.exists()) {
        return UNKNOWN;
    }
    if(info.isDir()) {
        return DIRECTORY;
    }
    QString mimetype = getMimeType(info);
    if(mimetype.startsWith("image")) {
        return IMAGE;
    } else if(mimetype == QObject::tr("application/zip")) {
        return ZIP;
    } else if(mimetype == QObject::tr("application/x-rar")) {
        return RAR;
    } else {
        qDebug() << "Mime detection failed, falling back onto file endings";
        if(filepath.endsWith(".pdf")) {
            return PDF;
        } else if(filepath.endsWith(".zip") || filepath.endsWith(".cbz")) {
            return ZIP;
            qDebug() << "Apparently its a zip";
        } else if(filepath.endsWith(".rar") || filepath.endsWith(".cbr")) {
            return RAR;
        } else if(isSupportedImageFormat(filepath.split(".").last())) {
            return IMAGE;
        }
    }
    qDebug() << "Don't know the file type, going with unknown...";
    return UNKNOWN;

}

