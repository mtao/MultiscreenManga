#ifndef CONFIG_H
#define CONFIG_H

#include <QStringList>
#include <QString>
#include <QImageReader>

/**
 * This should never be instantiated. Initialization happens in 
 * ConfigurationHidden's constructor, and a static instance of
 * ConfigurationHidden is embedded in Configuration (which is a
 * friend class).
 *
 * This class should have no public fields or methods.
 */
class ConfigurationHidden
{
public:
    friend class Configuration;
private:
    static ConfigurationHidden & getInstance();
    ConfigurationHidden();
    QStringList supportedImageFormats;
    QStringList supportedVolumeFormats;
    QStringList supportedFileFiltersList;
    QString supportedFileFilters;
};

class Configuration
{
private:
    ConfigurationHidden & config;
public:
    enum FileType {ZIP, RAR, PDF, IMAGE, DIRECTORY, UNKNOWN};
    Configuration();
    const QStringList & getSupportedImageFormats() const;
    const QStringList & getSupportedVolumeFormats() const;
    const QStringList & getSupportedFileFiltersList() const;
    const QString & getSupportedFileFilters() const;

    const bool isSupportedImageFormat(const QString & str) const;
    const bool isSupportedVolumeFormat(const QString & str) const;
    FileType getVolumeFormat(const QString & filename) const;
    static QString getMimeType(const QString& filename);
};

#endif
