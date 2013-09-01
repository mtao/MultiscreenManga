#ifdef USE_SERIALIZATION
#include "savestate.h"
#include "configuration.h"
#include <QFile>
#include <QCryptographicHash>
#include <fstream>
#include <QDebug>

SaveStateManager::SaveStateManager() {
    const QDir & dir = Configuration().getConfigurationDirectory();
    dir.mkpath("state");
    m_state_dir = QDir(dir.absoluteFilePath("state"));
}

const LocalSaveState& SaveStateManager::get_state(const std::string& path) {
    QCryptographicHash crypto(QCryptographicHash::Md5);
    QFile file(path.c_str());
    file.open(QFile::ReadOnly);
    while(!file.atEnd()) {
        crypto.addData(file.read(8192));
    }
    QByteArray hash = crypto.result().toHex();
    QString filename = file.fileName() + "-" + hash;
    

    std::string abspath(m_state_dir.absoluteFilePath(filename).toUtf8().data());
    if(m_state_dir.exists(filename)) {
        std::fstream input(abspath, std::ios::in | std::ios::binary);
        if(m_local_state.ParseFromIstream(&input)) {
            return m_local_state;
        } else {
            qDebug() << "Could not open local state file while creating a new file";
        }

    }
    m_local_state.set_path(abspath);
    SaveState & state = *m_local_state.mutable_state();
    state.set_filename(file.fileName().toUtf8().data());
    state.set_hash(hash.data());
    state.set_currentpage(0);
    save_state();
    return m_local_state;

};
void SaveStateManager::save_state() const {
    const SaveState & state = m_local_state.state();
    const QString config_filename((state.filename() + "-" + state.hash()).c_str());
    std::string abspath(m_state_dir.absoluteFilePath(config_filename).toUtf8().data());
    std::fstream output(abspath, std::ios::out | std::ios::binary);
    m_local_state.SerializeToOstream(&output);

}

void SaveStateManager::set_page(int idx) {
    m_local_state.mutable_state()->set_currentpage(idx);

}

#endif
