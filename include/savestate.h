#ifdef USE_SERIALIZATION
#ifndef SAVESTATE_H
#define SAVESTATE_H
#include "savestate.pb.h"
#include <QDir>
class SaveStateManager {
    public:
    SaveStateManager();
    const LocalSaveState& get_state(const std::string& str) ;
    void save_state() const;
    void set_page(int idx);
    private:
    QDir m_state_dir;
    LocalSaveState m_local_state;
};

#endif
#endif
