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
RemoteSyncServer::~RemoteSyncServer() {
    m_socket.unbind("tcp://*:8086");
}

void RemoteSyncServer::run() {

    SaveStateRPC rpc;
    std::string str;
    while(1) {
        if(m_poller.poll()) {
            if(m_poller.has_input(m_socket)) {
                //Read in rpc'd state
                m_socket.receive(str);
                rpc.ParseFromString(str);
                SaveState& state = *rpc.mutable_state();
                std::string filename(stateFilenameFromSaveState(state));

                SaveStateLock lock;
                int local_page = m_state_mgr.get_state(state).currentpage();
                int latest_page = std::max(state.currentpage(),local_page);
                std::cout << "Request made on " << state.filename() << ", they have page " << state.currentpage() << " and we have " << local_page << ".  ";
                //If we're forcing a smaller state, then latest_page has to be this
                if(rpc.forced()) {
                    latest_page = state.currentpage();
                    std::cout << "[Forced update]" << std::endl;
                } else {
                    std::cout << std::endl;
                }
                if(latest_page !=local_page) { 
                    m_state_mgr.set_page(latest_page);
                    m_state_mgr.save_state(false);
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
