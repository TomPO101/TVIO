#include "Network.h"
#include "HashGenerator.h"

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/beast/ssl.hpp>

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;

namespace TradingView {
namespace IO {

std::mutex Network::m_RegMutex; 
std::mutex Network::m_UnRegMutex;
std::mutex Network::m_GetSocketMutex;
Log Network::m_Log("TVNetwork");
net::io_context Network::m_Ioc;
ssl::context Network::m_Ctx{ ssl::context::tlsv12_client };
tcp::resolver Network::m_Resolver{ m_Ioc };
std::vector<std::shared_ptr<Socket>> Network::m_Sockets;
std::vector<int> Network::m_RegisteredIDs;
volatile bool Network::m_SocketBeingRemoved = false;

struct Socket
{
    int _ID;
    websocket::stream<beast::ssl_stream<tcp::socket>> _Socket{ Network::m_Ioc, Network::m_Ctx };
};

const bool Network::Register(int& sessionID)
{
    m_RegMutex.lock();

    ThreadWait();

    RegisterSocket(sessionID);

    const auto conResult = Connect(m_Sockets.back());

    m_RegMutex.unlock();

    return conResult;
}

void Network::RegisterSocket(int& sessionID)
{
    m_Sockets.emplace_back(std::make_shared<Socket>());

    GenerateID(sessionID);

    m_Sockets.back()->_ID = sessionID;
}

const bool Network::Connect(const std::shared_ptr<Socket>& sockPtr)
{
    beast::error_code ec;

    auto& socket = sockPtr->_Socket;

    auto const results = m_Resolver.resolve(TV_HOST, TV_PORT, ec);
    if (ec) {
        m_Log.Add(LOG_TVIO_ERROR, "Resolve Error", ec.message());
        return DisplayError(L"Resolve Error");
    }

    net::connect(socket.next_layer().next_layer(), results.begin(), results.end(), ec);
    if (ec) {
        m_Log.Add(LOG_TVIO_ERROR, "Connect Error", ec.message());
        return DisplayError(L"Connect Error");
    }

    SSL_set_tlsext_host_name(socket.next_layer().native_handle(), TV_HOST);

    socket.next_layer().handshake(ssl::stream_base::client, ec);
    if (ec) {
        m_Log.Add(LOG_TVIO_ERROR, "SSL Handshake Error", ec.message());
        return DisplayError(L"SSL Handshake Error");
    }

    socket.set_option(websocket::stream_base::decorator(
        [](websocket::request_type& req)
        {
            req.set(http::field::host, "prodata.tradingview.com");
            req.set(http::field::user_agent, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/83.0.4103.116 Safari/537.36");
            req.set(http::field::pragma, "no-cache");
            req.set(http::field::cache_control, "no-cache");
            req.set(http::field::upgrade, "websocket");
            req.set(http::field::origin, "https://uk.tradingview.com");
            req.set(http::field::sec_websocket_version, "13");
            req.set(http::field::accept_encoding, "gzip, deflate, br");
            req.set(http::field::accept_language, "en-GB,en-US;q=0.9,en;q=0.8");
        }));

    websocket::response_type response;
    std::ostringstream hostFormated;
    hostFormated << TV_HOST << ":" << TV_PORT;
    socket.handshake(response, hostFormated.str(), "/socket.io/websocket", ec);
    if (ec) {
        m_Log.Add(LOG_TVIO_ERROR, "Handshake Error", ec.message());
        return DisplayError(L"Handshake Error");
    }

    return true;
}

void Network::GenerateID(int& sessionID)
{
    int ID = 0;

    while (std::find(m_RegisteredIDs.begin(), m_RegisteredIDs.end(), ID) != m_RegisteredIDs.end()) {
        ID++;
    }

    sessionID = ID;
    m_RegisteredIDs.push_back(ID);
}

void Network::ThreadWait(void)
{
    size_t size = m_Sockets.size();
    while (size >= (size_t)MAX_CONSECUTIVE_SESSIONS) {

        //DisplayError(std::to_wstring(size).c_str());

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        size = m_Sockets.size();
    }
}

std::shared_ptr<Socket>* Network::GetSocket(const int& sessionID)
{
    std::scoped_lock<std::mutex> lock(m_GetSocketMutex);

    while (m_SocketBeingRemoved)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    for (auto& socket : m_Sockets)
        if (socket->_ID == sessionID)
            return &socket;

    return nullptr;
}

const bool Network::Write(const std::string& message, const int& sessionID)
{
    beast::error_code ec;

    auto* socket = GetSocket(sessionID);
    if (!socket)
        return DisplayError(L"Failed To Get Socket In Write");

    (*socket)->_Socket.write(net::buffer(message), ec);

    if (ec) {
        m_Log.Add(LOG_TVIO_ERROR, "Failed To Write", message);
        m_Log.Add(LOG_TVIO_ERROR, "Write Error", ec.message());

        return DisplayError(L"Failed To Write");
    }

    return true;
}

const bool Network::Read(std::string& messageBuffer, const int& sessionID)
{
    beast::error_code ec;
    beast::flat_buffer buffer;

    auto* socket = GetSocket(sessionID);
    if (!socket)
        return DisplayError(L"Failed To Get Socket In Read");

    (*socket)->_Socket.read(buffer, ec);

    if (ec) {
        m_Log.Add(LOG_TVIO_ERROR, "Read Error", ec.message());

        return DisplayError(L"Read Error");
    }

    messageBuffer = boost::beast::buffers_to_string(buffer.data());

    return true;
}

const bool Network::UnRegister(const int& sessionID)
{
    std::scoped_lock<std::mutex> lock(m_UnRegMutex);

    auto* socket = GetSocket(sessionID);
    if (!socket)
        return DisplayError(L"Failed To Get Socket In UnRegister");

    beast::error_code ec;

    (*socket)->_Socket.close(websocket::close_code::normal, ec);

    if (ec) {
        m_Log.Add(LOG_TVIO_ERROR, "Disconnect Error", ec.message());
        return DisplayError(L"Disconnect Error");
    }

    for (auto it = m_Sockets.begin(); it != m_Sockets.end(); it++)
        if ((*it)->_ID == sessionID) {
            m_SocketBeingRemoved = true;
            m_Sockets.erase(it);
            m_SocketBeingRemoved = false;
            break;
        }

    //DisplayError(std::to_wstring(m_Sockets.size()).c_str());

    return true;
}

const bool Network::DisplayError(const std::wstring& error)
{
    std::wstringstream errorStream;
    errorStream << "ThreadID: " << std::this_thread::get_id() << " | ";
    errorStream << "Error: " << error.c_str() << std::endl;

    OutputDebugString((LPCWSTR)errorStream.str().c_str());

    std::wcout << errorStream.str() << std::endl;

    return false;
}

//volatile bool Network::m_Initialised = false;
//std::mutex Network::m_RegMutex;
//std::mutex Network::m_UnRegMutex;
//Log Network::m_Log("TVNetwork");
//net::io_context Network::m_Ioc;
//ssl::context Network::m_Ctx{ ssl::context::tlsv12_client };
//tcp::resolver Network::m_Resolver{ m_Ioc };
//std::vector<std::shared_ptr<Socket>> Network::m_Sockets;
//std::vector<int> Network::m_RegisteredIDs;
//
//struct Socket
//{
//    /*std::atomic<int> _ID = SOCKET_UNASSIGNED;
//    std::atomic<bool> _InUse = false;*/
//    int _ID = SOCKET_UNASSIGNED;
//    bool _InUse = false;
//    websocket::stream<beast::ssl_stream<tcp::socket>> _Socket{ Network::m_Ioc, Network::m_Ctx };
//};
//
//void Network::Init(void)
//{
//    if (m_Initialised) return;
//
//    //DisplayError(L"In Init");
//
//    //m_Sockets.reserve(MAX_CONSECUTIVE_SESSIONS);
//
//    for (int i = 0; i < MAX_CONSECUTIVE_SESSIONS; i++) {
//
//        m_Sockets.emplace_back(std::make_shared<Socket>());
//
//        
//    }
//
//    m_Initialised = true;
//}
//
//const bool Network::Register(int& sessionID)
//{
//    //DisplayError(L"Before Lock");
//
//    m_RegMutex.lock();
//
//    //DisplayError(L"After Lock");
//
//    Init();
//
//    WaitForSocket(sessionID);
//
//    m_RegMutex.unlock();
//
//    //DisplayError(L"After UnLock");
//
//    //return conResult;
//    return true;
//}
//
//const bool Network::Connect(const std::shared_ptr<Socket>& sockPtr)
//{
//    beast::error_code ec;
//
//    auto& socket = sockPtr->_Socket;
//
//    auto const results = m_Resolver.resolve(TV_HOST, TV_PORT, ec);
//    if (ec) {
//        m_Log.Add(LOG_TVIO_ERROR, "Resolve Error", ec.message());
//        return DisplayError(L"Resolve Error");
//    }
//
//    net::connect(socket.next_layer().next_layer(), results.begin(), results.end(), ec);
//    if (ec) {
//        m_Log.Add(LOG_TVIO_ERROR, "Connect Error", ec.message());
//        return DisplayError(L"Connect Error");
//    }
//
//    SSL_set_tlsext_host_name(socket.next_layer().native_handle(), TV_HOST);
//
//    socket.next_layer().handshake(ssl::stream_base::client, ec);
//    if (ec) {
//        m_Log.Add(LOG_TVIO_ERROR, "SSL Handshake Error", ec.message());
//        return DisplayError(L"SSL Handshake Error");
//    }
//
//    socket.set_option(websocket::stream_base::decorator(
//        [](websocket::request_type& req)
//        {
//            req.set(http::field::host, "prodata.tradingview.com");
//            req.set(http::field::user_agent, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/83.0.4103.116 Safari/537.36");
//            req.set(http::field::pragma, "no-cache");
//            req.set(http::field::cache_control, "no-cache");
//            req.set(http::field::upgrade, "websocket");
//            req.set(http::field::origin, "https://uk.tradingview.com");
//            req.set(http::field::sec_websocket_version, "13");
//            req.set(http::field::accept_encoding, "gzip, deflate, br");
//            req.set(http::field::accept_language, "en-GB,en-US;q=0.9,en;q=0.8");
//        }));
//
//    websocket::response_type response;
//    std::ostringstream hostFormated;
//    hostFormated << TV_HOST << ":" << TV_PORT;
//    socket.handshake(response, hostFormated.str(), "/socket.io/websocket", ec);
//    if (ec) {
//        m_Log.Add(LOG_TVIO_ERROR, "Handshake Error", ec.message());
//        return DisplayError(L"Handshake Error");
//    }
//
//    return true;
//}
//
//void Network::GenerateID(int& sessionID)
//{
//    int ID = 0;
//
//    while (std::find(m_RegisteredIDs.begin(), m_RegisteredIDs.end(), ID) != m_RegisteredIDs.end()) {
//        ID++;
//    }
//
//    sessionID = ID;
//    m_RegisteredIDs.push_back(ID);
//}
//
//void Network::WaitForSocket(int& sessionID)
//{
//    while (true) {
//
//        for (auto& socket : m_Sockets)
//            if (!socket->_InUse) {
//
//
//
//                GenerateID(socket->_ID);
//                socket->_InUse = true;
//                sessionID = socket->_ID;
//
//                
//
//                const auto conResult = Connect(socket);
//
//                return;
//            }
//    }
//}
//
//std::shared_ptr<Socket>* Network::GetSocket(const int& sessionID)
//{
//    for (auto& socket : m_Sockets)
//        if (socket->_ID == sessionID)
//            return &socket;
//
//    return nullptr;
//}
//
//const bool Network::Write(const std::string& message, const int& sessionID)
//{
//    beast::error_code ec;
//
//    auto* socket = GetSocket(sessionID);
//    if (!socket)
//        return DisplayError(L"Failed To Get Socket In Write");
//
//    (*socket)->_Socket.write(net::buffer(message), ec);
//
//    if (ec) {
//        m_Log.Add(LOG_TVIO_ERROR, "Failed To Write", message);
//        m_Log.Add(LOG_TVIO_ERROR, "Write Error", ec.message());
//
//        return DisplayError(L"Failed To Write");
//    }
//
//    return true;
//}
//
//const bool Network::Read(std::string& messageBuffer, const int& sessionID)
//{
//    beast::error_code ec;
//    beast::flat_buffer buffer;
//
//    auto* socket = GetSocket(sessionID);
//    if (!socket)
//        return DisplayError(L"Failed To Get Socket In Read");
//
//    (*socket)->_Socket.read(buffer, ec);
//
//    if (ec) {
//        m_Log.Add(LOG_TVIO_ERROR, "Read Error", ec.message());
//
//        return DisplayError(L"Read Error");
//    }
//
//    messageBuffer = boost::beast::buffers_to_string(buffer.data());
//
//    return true;
//}
//
//const bool Network::UnRegister(const int& sessionID)
//{
//    std::scoped_lock<std::mutex> lock(m_UnRegMutex);
//
//    auto* socket = GetSocket(sessionID);
//    if (!socket)
//        return DisplayError(L"Failed To Get Socket In UnRegister");
//
//    beast::error_code ec;
//
//    (*socket)->_Socket.close(websocket::close_code::normal, ec);
//
//    if (ec) {
//        m_Log.Add(LOG_TVIO_ERROR, "Disconnect Error", ec.message());
//        DisplayError(L"Disconnect Error");
//    }
//
//    (*socket)->_ID = SOCKET_UNASSIGNED;
//    (*socket)->_InUse = false;
//
//    return true;
//}
//
//const bool Network::DisplayError(const std::wstring& error)
//{
//    std::wstringstream errorStream;
//    errorStream << "ThreadID: " << std::this_thread::get_id() << " | ";
//    errorStream << "Error: " << error.c_str() << std::endl;
//
//    OutputDebugString((LPCWSTR)errorStream.str().c_str());
//
//    std::wcout << errorStream.str() << std::endl;
//
//    return false;
//}
//
//const bool Network::Finish(void)
//{
//    beast::error_code ec;
//
//    for (const auto& socket : m_Sockets) {
//
//        socket->_Socket.close(websocket::close_code::normal, ec);
//
//        if (ec) {
//            m_Log.Add(LOG_TVIO_ERROR, "Disconnect Error", ec.message());
//            return DisplayError(L"Disconnect Error");
//        }
//    }
//}

}
}