#ifdef USE_SERIALIZATION
#ifndef SAVESTATE_H
#define SAVESTATE_H
#include "savestate.pb.h"
#include <QDir>
class SaveStateManager {
    SaveStateManager();
    LocalSaveState get_state(const std::string& str) const;
    void save_state(const LocalSaveState & local_state) const;
    private:
    QDir state_dir;
};

#endif
#endif
