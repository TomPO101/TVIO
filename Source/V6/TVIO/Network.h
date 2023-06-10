#pragma once
#include "Log/Log.h"

#define BOOST_ASIO_HAS_MOVE 1

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00 
#endif

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <iostream>
#include <functional>
#include <string>

using namespace TDPGeneral;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

namespace TradingView {
namespace IO {

#define AUTH_KEY ("eyJ0eXAiOiJKV1QiLCJhbGciOiJSUzUxMiJ9.eyJ1c2VyX2lkIjo1NjM1MDExLCJleHAiOjE1OTk4Mzg1NDcsImlhdCI6MTU5OTgyNDE0NywicGxhbiI6InByb19wcmVtaXVtIiwiZXh0X2hvdXJzIjoxLCJwZXJtIjoiYW1leCxueXNlLG5hc2RhcSIsInN0dWR5X3Blcm0iOiJ0di1wcm9zdHVkaWVzLHR2LXZvbHVtZWJ5cHJpY2UiLCJtYXhfc3R1ZGllcyI6MjUsIm1heF9mdW5kYW1lbnRhbHMiOjB9.W6PfOPMsVfcivyjePZ0zJBEPRSMfXyNh7yd3tZZsoAbyW8WRGMeclYuBfLpvBorspmZilNCBQKRebQN7eXP5SUWvmdd4J7aqqGR7uY6ejVIllToYoyBcwgsQY2wJQ7peEPdRtfc5g9240vnWap4-CfAFujBbW8bJ1o19DKe2ufU")
#define TV_HOST ("prodata.tradingview.com")
#define TV_PORT ("443")
#define MAX_CONSECUTIVE_SESSIONS (5)

struct Socket;
	
class Network
{
	friend struct Socket;
	
public:
	static const bool Register(int& sessionID);
	static const bool UnRegister(const int& sessionID);
	
	static const bool Write(const std::string& message, const int& sessionID);
	static const bool Read(std::string& messageBuffer, const int& sessionID);
	
private:
	static void RegisterSocket(int& sessionID);
	static const bool Connect(const std::shared_ptr<Socket>& sockPtr);
	static std::shared_ptr<Socket>* GetSocket(const int& sessionID);
	static void GenerateID(int& sessionID);
	static void ThreadWait(void);
	static const bool DisplayError(const std::wstring& error);
	
private:
	static std::mutex m_RegMutex, m_UnRegMutex, m_GetSocketMutex;
	static Log m_Log;
	static net::io_context m_Ioc;
	static ssl::context m_Ctx;
	static tcp::resolver m_Resolver;
	static std::vector<std::shared_ptr<Socket>> m_Sockets;
	static std::vector<int> m_RegisteredIDs;
	volatile static bool m_SocketBeingRemoved;
};


//#define AUTH_KEY ("eyJ0eXAiOiJKV1QiLCJhbGciOiJSUzUxMiJ9.eyJ1c2VyX2lkIjo1NjM1MDExLCJleHAiOjE1OTk4Mzg1NDcsImlhdCI6MTU5OTgyNDE0NywicGxhbiI6InByb19wcmVtaXVtIiwiZXh0X2hvdXJzIjoxLCJwZXJtIjoiYW1leCxueXNlLG5hc2RhcSIsInN0dWR5X3Blcm0iOiJ0di1wcm9zdHVkaWVzLHR2LXZvbHVtZWJ5cHJpY2UiLCJtYXhfc3R1ZGllcyI6MjUsIm1heF9mdW5kYW1lbnRhbHMiOjB9.W6PfOPMsVfcivyjePZ0zJBEPRSMfXyNh7yd3tZZsoAbyW8WRGMeclYuBfLpvBorspmZilNCBQKRebQN7eXP5SUWvmdd4J7aqqGR7uY6ejVIllToYoyBcwgsQY2wJQ7peEPdRtfc5g9240vnWap4-CfAFujBbW8bJ1o19DKe2ufU")
//#define TV_HOST ("prodata.tradingview.com")
//#define TV_PORT ("443")
//#define MAX_CONSECUTIVE_SESSIONS (6)
//#define SOCKET_UNASSIGNED (-2)
//
//struct Socket;
//
//class Network
//{
//	friend struct Socket;
//
//public:
//	static const bool Register(int& sessionID);
//	static const bool UnRegister(const int& sessionID);
//
//	static const bool Write(const std::string& message, const int& sessionID);
//	static const bool Read(std::string& messageBuffer, const int& sessionID);
//
//private:
//	static void Init(void);
//	static void WaitForSocket(int& sessionID);
//	static const bool Connect(const std::shared_ptr<Socket>& sockPtr);
//	static std::shared_ptr<Socket>* GetSocket(const int& sessionID);
//	static void GenerateID(int& sessionID);
//	static const bool DisplayError(const std::wstring& error);
//	static const bool Finish(void);
//
//private:
//	volatile static bool m_Initialised;
//	static std::mutex m_RegMutex, m_UnRegMutex;
//	static Log m_Log;
//	static net::io_context m_Ioc;
//	static ssl::context m_Ctx;
//	static tcp::resolver m_Resolver;
//	static std::vector<std::shared_ptr<Socket>> m_Sockets;
//	static std::vector<int> m_RegisteredIDs;
//};

}
} 