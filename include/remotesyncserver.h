#ifdef USE_NETWORKING
#ifndef REMOTE_SYNC_SERVER_H
#define REMOTE_SYNC_SERVER_H
#include <zmqpp/zmqpp.hpp>
#include "savestate.h"

class RemoteSyncServer {
    public:
        RemoteSyncServer();
        ~RemoteSyncServer();
        void run();
    private:
        zmqpp::context m_context;
        zmqpp::socket m_socket;
        zmqpp::poller m_poller;
        SaveStateManager m_state_mgr;
};

#endif
#endif
