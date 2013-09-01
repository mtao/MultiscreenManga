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
    state_dir = QDir(dir.absoluteFilePath("state"));
}

LocalSaveState SaveStateManager::get_state(const std::string& path) const {
    QCryptographicHash crypto(QCryptographicHash::Sha1);
    QFile file(path.c_str());
    file.open(QFile::ReadOnly);
    while(!file.atEnd()) {
        crypto.addData(file.read(8192));
    }
    QByteArray hash = crypto.result();
    QString filename = file.fileName() + "-" + hash;
    
    LocalSaveState local_state;
    SaveState & state = *local_state.mutable_state();

    std::string abspath(state_dir.absoluteFilePath(filename).toUtf8().data());
    bool writeStateInfo = false;
    if(state_dir.exists(filename)) {
        std::fstream input(abspath, std::ios::in | std::ios::binary);
        if(local_state.ParseFromIstream(&input)) {
            return local_state;
        } else {
            qDebug() << "Could not open local state file while creating a new file";
            writeStateInfo = true;
        }

    }
    if(writeStateInfo) {
        local_state.set_path(abspath);
        state.set_filename(file.fileName().toUtf8().data());
        state.set_hash(hash.data());
    }
    return local_state;

};
void SaveStateManager::save_state(const LocalSaveState & local_state) const {
    const SaveState & state = local_state.state();
    const QString config_filename((state.filename() + "-" + state.hash()).c_str());
    std::string abspath(state_dir.absoluteFilePath(config_filename).toUtf8().data());
    std::fstream output(abspath, std::ios::out | std::ios::binary);
    local_state.SerializeToOstream(&output);

}

#endif
