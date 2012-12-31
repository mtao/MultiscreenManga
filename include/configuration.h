#ifndef CONFIG_H
#define CONFIG_H

#include <QList>
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
    QList<QString> supportedImageFormats;
    QList<QString> supportedVolumeFormats;
    QString supportedFileFilters;
};

class Configuration
{
private:
    ConfigurationHidden & config;
public:
    Configuration();
    const QList<QString> & getSupportedImageFormats() const;
    const QList<QString> & getSupportedVolumeFormats() const;
    const QString & getSupportedFileFilters() const;
};

#endif
