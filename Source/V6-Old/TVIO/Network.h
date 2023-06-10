#pragma once
#include "Log/Log.h"
#define BOOST_ASIO_HAS_MOVE 1

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/thread/thread.hpp>
#include <iostream>
#include <string>
#include <functional>

using namespace TDPGeneral;
namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

namespace TradingView {
namespace IO {

#define AUTH_KEY ("eyJ0eXAiOiJKV1QiLCJhbGciOiJSUzUxMiJ9.eyJ1c2VyX2lkIjo1NjM1MDExLCJleHAiOjE1OTk4Mzg1NDcsImlhdCI6MTU5OTgyNDE0NywicGxhbiI6InByb19wcmVtaXVtIiwiZXh0X2hvdXJzIjoxLCJwZXJtIjoiYW1leCxueXNlLG5hc2RhcSIsInN0dWR5X3Blcm0iOiJ0di1wcm9zdHVkaWVzLHR2LXZvbHVtZWJ5cHJpY2UiLCJtYXhfc3R1ZGllcyI6MjUsIm1heF9mdW5kYW1lbnRhbHMiOjB9.W6PfOPMsVfcivyjePZ0zJBEPRSMfXyNh7yd3tZZsoAbyW8WRGMeclYuBfLpvBorspmZilNCBQKRebQN7eXP5SUWvmdd4J7aqqGR7uY6ejVIllToYoyBcwgsQY2wJQ7peEPdRtfc5g9240vnWap4-CfAFujBbW8bJ1o19DKe2ufU")

class Network
{
	friend class Session;
	friend class Inbox;
	friend class Outbox;

private:
	Network(Log& log, const int sessionID);
	bool ConnectWithProxy(void);
	bool Connect(void);
	bool Disconnect(void);

	bool Write(const std::string& message);
	bool Read(std::string& messageBuffer);

private:
	Log& m_Log;
	const int m_SessionID;
	net::io_context m_Ioc;
	ssl::context m_Ctx;
	tcp::resolver m_Resolver;
	websocket::stream<beast::ssl_stream<tcp::socket>> m_Ws;
	std::string m_Host;
	std::string m_Proxy;
	std::string m_Port;
};

}
}