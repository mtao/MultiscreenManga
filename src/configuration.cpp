#include "include/configuration.h"

// ConfigurationHidden

ConfigurationHidden::ConfigurationHidden()
{
  // Read in Qt's reported supported image formats
  QList<QByteArray> formatsBytes = QImageReader::supportedImageFormats();
  for (auto && format: formatsBytes) {
    supportedImageFormats.push_back(QString(format));
  }
}

ConfigurationHidden & ConfigurationHidden::getInstance() {
    static ConfigurationHidden * ch = new ConfigurationHidden();
    return *ch;
}

// Configuration

Configuration::Configuration(): config(ConfigurationHidden::getInstance()) {}

const QList<QString> & Configuration::getSupportedImageFormats() const {
  return config.supportedImageFormats;
}
