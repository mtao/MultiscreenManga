#include "include/configuration.h"

// ConfigurationHidden

ConfigurationHidden::ConfigurationHidden()
{
  // Read in Qt's reported supported image formats
  QList<QByteArray> formatsBytes = QImageReader::supportedImageFormats();
  for (int i=0; i<formatsBytes.size(); i++) {
    const QString* str = new QString(formatsBytes.at(i));
    supportedImageFormats.push_back(*str);
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
