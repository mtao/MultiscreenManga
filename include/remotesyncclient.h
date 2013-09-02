#ifdef USE_NETWORKING
#ifndef REMOTE_SYNCCLIENT_H
#define REMOTE_SYNCCLIENT_H
#include <zmqpp/zmqpp.hpp>
#include "savestate.h"

class RemoteSyncClient {
    public:
        RemoteSyncClient();
        SaveState& sync(const std::string& filename);
        SaveState& sync();
        SaveStateManager& manager() { return m_state_mgr; }
    private:
        zmqpp::context m_context;
        zmqpp::socket m_socket;
        SaveStateManager m_state_mgr;
        bool active;
};

#endif
#endif
