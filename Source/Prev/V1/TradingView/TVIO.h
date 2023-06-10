#pragma once
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
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

class TVIO
{
public:
	TVIO(void);
	~TVIO(void);

	bool Connect(void);
	bool Disconnect(void);

	net::io_context& GetIOC(void) { return m_Ioc; };
	ssl::context& GetCtx(void) { return m_Ctx; };

	bool Write(std::string message);
	std::string Read(void);
	void AddAsyncReadCallback(std::function<void(std::string)> callback) { m_AsyncReadCallback = callback; };
	void AsyncRead(boost::system::error_code ec, std::size_t bytes=NULL);

private:
	net::io_context m_Ioc;
	ssl::context m_Ctx;
	tcp::resolver m_Resolver;
	websocket::stream<beast::ssl_stream<tcp::socket>> m_Ws;
	std::string m_Host;
	std::string m_Port;
	std::function<void(std::string)> m_AsyncReadCallback;
};

}