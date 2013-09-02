#ifdef USE_NETWORKING
#include "remotesyncclient.h"
#include <iostream>


RemoteSyncClient::RemoteSyncClient()
    : m_context()
      , m_socket(m_context,zmqpp::socket_type::request)
{
    active = true;
    
}
SaveState& RemoteSyncClient::sync(const std::string& filename, bool forced) {
   m_state_mgr.get_state(filename);
   return sync(forced);
}
SaveState& RemoteSyncClient::sync(bool forced) {
    SaveState& state = m_state_mgr.get_state();
    if(!active) {
        return state;
    }
    SaveStateRPC rpc;
    rpc.set_forced(forced);
    SaveState& rpc_state = *rpc.mutable_state();
    rpc_state = state;
    rpc_state.set_currentpage(state.currentpage());
    rpc_state.set_filename(state.filename());
    rpc_state.set_hash(state.hash());
    std::string endpoint("tcp://192.168.1.5:8086");
    try {
        m_socket.connect(endpoint);

    } catch(zmqpp::zmq_internal_exception& e) {
        std::cerr << "Remote server is down, just using local information" << std::endl;
        return state;
    }

    std::string str;
    rpc.SerializeToString(&str);
    zmqpp::message message;
    message.add(str.data(), str.size());
    m_socket.send(message,true);
    zmqpp::poller poller;
    poller.add(m_socket);
    if(poller.poll(500)) {
        if(poller.has_input(m_socket)) {
            m_socket.receive(str);
            state.ParseFromString(str);
        }
    } else {
        active = false;
    }
    m_socket.disconnect(endpoint);
    return state;


}


#endif
