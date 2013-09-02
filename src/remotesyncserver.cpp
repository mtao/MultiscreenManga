#ifdef USE_NETWORKING
#include "remotesyncserver.h"
#include <iostream>
#include "savestate.h"

RemoteSyncServer::RemoteSyncServer()
: m_context()
    , m_socket(m_context,zmqpp::socket_type::reply)
{
    try {
        m_socket.bind("tcp://*:8086");
    } catch(zmqpp::zmq_internal_exception& e) {
        std::cerr << "failed to bind to endpoint: " << e.what() << std::endl;
        return;
    }
    m_poller.add(m_socket);

}

void RemoteSyncServer::run() {

    SaveState state;
    std::string str;
    while(1) {
        if(m_poller.poll()) {
            if(m_poller.has_input(m_socket)) {
                m_socket.receive(str);
                state.ParseFromString(str);
                std::string filename(stateFilenameFromSaveState(state));
                SaveStateLock lock;
                int local_page = m_state_mgr.get_state(state).currentpage();
                int latest_page = std::max(state.currentpage(),local_page);
                std::cout << "Request made on " << state.filename() << ", they have page " << state.currentpage() << " and we have " << local_page << std::endl;
                if(latest_page !=local_page) { 
                    m_state_mgr.set_page(latest_page);
                } else {
                    state.set_currentpage(latest_page);
                }
                state.SerializeToString(&str);
                zmqpp::message message;
                message.add(str.data(),str.size());
                m_socket.send(message,true);

            }
        }
    }
}

#endif
