#pragma once
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

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

namespace TradingView {

class Log;

#define AUTH_KEY ("eyJ0eXAiOiJKV1QiLCJhbGciOiJSUzUxMiJ9.eyJ1c2VyX2lkIjo1NjM1MDExLCJleHAiOjE1OTk4Mzg1NDcsImlhdCI6MTU5OTgyNDE0NywicGxhbiI6InByb19wcmVtaXVtIiwiZXh0X2hvdXJzIjoxLCJwZXJtIjoiYW1leCxueXNlLG5hc2RhcSIsInN0dWR5X3Blcm0iOiJ0di1wcm9zdHVkaWVzLHR2LXZvbHVtZWJ5cHJpY2UiLCJtYXhfc3R1ZGllcyI6MjUsIm1heF9mdW5kYW1lbnRhbHMiOjB9.W6PfOPMsVfcivyjePZ0zJBEPRSMfXyNh7yd3tZZsoAbyW8WRGMeclYuBfLpvBorspmZilNCBQKRebQN7eXP5SUWvmdd4J7aqqGR7uY6ejVIllToYoyBcwgsQY2wJQ7peEPdRtfc5g9240vnWap4-CfAFujBbW8bJ1o19DKe2ufU")

class TVIO
{
public:
	TVIO(TradingView::Log& log, const unsigned int sessionID);
	~TVIO(void);

	bool ConnectWithProxy(void);
	bool Connect(void);
	bool Disconnect(void);

	net::io_context& GetIOC(void) { return m_Ioc; };
	ssl::context& GetCtx(void) { return m_Ctx; };

	bool Write(const std::string& message);
	bool Read(std::string& messageBuffer);

private:
	TradingView::Log& m_Log;
	const unsigned int m_SessionID;
	std::mutex m_Mutex;	
	net::io_context m_Ioc;
	ssl::context m_Ctx;
	tcp::resolver m_Resolver;
	websocket::stream<beast::ssl_stream<tcp::socket>> m_Ws;
	std::string m_Host;
	std::string m_Proxy;
	std::string m_Port;
};

}