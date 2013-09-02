#ifdef USE_SERIALIZATION
#ifndef SAVESTATE_H
#define SAVESTATE_H
#include "savestate.pb.h"
#include <QDir>

inline std::string stateFilenameFromSaveState(const SaveState& state) {
    return state.filename() + "-" + state.hash();
}

class SaveStateLock {
    public:
        SaveStateLock();
        ~SaveStateLock();
    private:
        static const std::string lockpath;
        int fd;
};


class SaveStateManager {
    public:
    SaveStateManager();
    SaveState& get_state(const std::string& str) ;
    SaveState& get_state(const std::string& filename, const std::string& hash) ;
    SaveState& get_state(const SaveState& state) ;
    SaveState& get_state() {return m_state;}
    void save_state(bool do_lock=true) const;
    void set_page(int idx);
    private:
    QDir m_state_dir;
    SaveState m_state;
};

#endif
#endif
