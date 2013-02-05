#include "include/configuration.h"

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
