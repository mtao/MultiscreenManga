#ifdef USE_SERIALIZATION
#include "savestate.h"
#include "configuration.h"
#include <QFile>
#include <QCryptographicHash>
#include <fstream>
#include <QDebug>
#include <fcntl.h>
#include <unistd.h>
#include <memory>


const std::string SaveStateLock::lockpath = "/tmp/multiscreenmanga.lock";

SaveStateLock::SaveStateLock() {
    do {
        fd = open(lockpath.c_str(), O_EXCL | O_CREAT | O_WRONLY);
    } while (fd == -1);
    close(fd);
}
SaveStateLock::~SaveStateLock() {
    remove(lockpath.c_str());
}

SaveStateManager::SaveStateManager() {
    const QDir & dir = Configuration().getConfigurationDirectory();
    dir.mkpath("state");
    m_state_dir = QDir(dir.absoluteFilePath("state"));
}

SaveState& SaveStateManager::get_state(const SaveState & state) {
    return get_state(state.filename(),state.hash());
}
SaveState& SaveStateManager::get_state(const std::string& filename, const std::string& hash) {
    //meh i usually end up doing a double conversion but it makes the code simpler
    QString config_filename((filename + "-" + hash).c_str());
    std::string abspath(m_state_dir.absoluteFilePath(config_filename).toUtf8().data());
    if(m_state_dir.exists(config_filename)) {
        std::fstream input(abspath, std::ios::in | std::ios::binary);
        if(m_state.ParseFromIstream(&input)) {
            return m_state;
        } else {
            qDebug() << "Could not open local state file while creating a new file";
        }

    }
    m_state.set_filename(filename);
    m_state.set_hash(hash);
    m_state.set_currentpage(0);
    return m_state;
}
SaveState& SaveStateManager::get_state(const std::string& path) {
    QCryptographicHash crypto(QCryptographicHash::Md5);
    QFile file(path.c_str());
    file.open(QFile::ReadOnly);
    while(!file.atEnd()) {
        crypto.addData(file.read(8192));
    }
    QByteArray hash = crypto.result().toHex();
    get_state(file.fileName().toUtf8().data(),hash.data());
    

    return m_state;

};
void SaveStateManager::save_state(bool do_lock) const {
    std::unique_ptr<SaveStateLock> lock;
    if(do_lock) {
        lock.reset(new SaveStateLock());
    }
    const QString config_filename(stateFilenameFromSaveState(m_state).c_str());
    std::string abspath(m_state_dir.absoluteFilePath(config_filename).toUtf8().data());
    std::fstream output(abspath, std::ios::out | std::ios::binary);
    m_state.SerializeToOstream(&output);

}

void SaveStateManager::set_page(int idx) {
    m_state.set_currentpage(idx);

}

#endif
