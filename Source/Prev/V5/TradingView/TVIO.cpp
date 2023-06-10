#include "TVIO.h"
#include "Log.h"

TradingView::TVIO::TVIO(TradingView::Log& log, const unsigned int sessionID) :
    m_Log(log),
    m_SessionID(sessionID),
	m_Ioc(),
	m_Ctx{ ssl::context::tlsv12_client },
	m_Resolver{ m_Ioc },
    m_Ws{ m_Ioc, m_Ctx },
	m_Host("prodata.tradingview.com"),
    m_Proxy("196.17.176.183:51996"),
	m_Port("443")
{
}

TradingView::TVIO::~TVIO(void)
{
}

bool TradingView::TVIO::ConnectWithProxy(void)
{
    beast::error_code ec;

    auto const results = m_Resolver.resolve("196.17.176.183", "51996");

    net::connect(m_Ws.next_layer().next_layer(), results.begin(), results.end());

    const boost::asio::ip::address clientAddr = m_Ws.next_layer().next_layer().local_endpoint().address();
    const boost::asio::ip::address serverAddr = m_Ws.next_layer().next_layer().remote_endpoint().address();

    std::cout << "ClientAddr: " << clientAddr << std::endl;
    std::cout << "ServerAddr: " << serverAddr << std::endl << std::endl;

    http::request<http::string_body> request_connect{ http::verb::connect, m_Host, 11 };
    //request_connect.insert(http::field::proxy_authorization, "Basic dXNlcm5hbWU6cGFzc3dvcmQ=");
    request_connect.insert(http::field::host, m_Host);
    request_connect.insert(http::field::proxy_connection, "keep-alive");
    request_connect.insert(http::field::connection, "keep-alive");
    boost::beast::http::write(m_Ws.next_layer().next_layer(), request_connect, ec);
    if (ec)
        std::cout << "WRITE ERROR: " << ec.message() << std::endl;

    http::response<http::empty_body> res;
    http::parser<false, http::empty_body> p(res);
    p.skip(true);
    boost::beast::flat_buffer buffer;
    http::read(m_Ws.next_layer().next_layer(), buffer, p, ec);
    std::cout << "READ: " << boost::beast::buffers_to_string(buffer.data()) << std::endl;
    if (ec)
        std::cout << "READ ERROR: " << ec.message() << std::endl;

    //SSL_set_tlsext_host_name(m_Ws.next_layer().native_handle(), m_Host.c_str());
    m_Ws.next_layer().handshake(ssl::stream_base::client, ec);
    if (ec)
        std::cout << "SSL HANDSHAKE ERROR: " << ec.message() << std::endl;

    m_Host += ':' + m_Port;

    m_Ws.set_option(websocket::stream_base::decorator(
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
            //req.set(http::field::sec_websocket_extensions, "permessage-deflate; client_max_window_bits");
        }));

    websocket::response_type response;
    //m_Ws.handshake(response, m_Host, "/socket.io/websocket?from=chart%2FyRSTdo04%2F&date=2020_07_22-12_23");
    m_Ws.handshake(response, m_Host, "/socket.io/websocket", ec);
    if (ec)
        std::cout << "WEB SOCKET HANDSHAKE ERROR: " << ec.message() << std::endl;

    std::cout << "RESPONSE BASE: " << response.base() << std::endl << std::endl;
    std::cout << "RESPONSE BODY: " << response.body() << std::endl;
    return true;
}

bool TradingView::TVIO::Connect(void)
{
    beast::error_code ec;
    auto const results = m_Resolver.resolve(m_Host, m_Port, ec);
    if (ec) {
        m_Log.Add(LOG_TVIO_ERROR, "Resolve Error", ec.message());
        return false;
    }

    net::connect(m_Ws.next_layer().next_layer(), results.begin(), results.end(), ec);
    if (ec) {
        m_Log.Add(LOG_TVIO_ERROR, "Connect Error", ec.message());
    }

    SSL_set_tlsext_host_name(m_Ws.next_layer().native_handle(), m_Host.c_str());
    m_Ws.next_layer().handshake(ssl::stream_base::client, ec);
    if (ec) {
        m_Log.Add(LOG_TVIO_ERROR, "SSL Handshake Error", ec.message());
        return false;
    }

    m_Host += ':' + m_Port;

    m_Ws.set_option(websocket::stream_base::decorator(
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
    m_Ws.handshake(response, m_Host, "/socket.io/websocket", ec);
    if (ec) {
        m_Log.Add(LOG_TVIO_ERROR, "Handshake Error", ec.message());
        return false;
    }

    return true;
}

bool TradingView::TVIO::Disconnect(void)
{
    beast::error_code ec;
    m_Ws.close(websocket::close_code::normal, ec);
    if (ec) {
        m_Log.Add(LOG_TVIO_ERROR, "Disconnect Error", ec.message());
    }
    return true;
}

bool TradingView::TVIO::Write(const std::string& message)
{
    beast::error_code ec;
    m_Ws.write(net::buffer(message), ec);
    if (ec) {
        m_Log.Add(LOG_TVIO_ERROR, "Failed To Write", message);
        m_Log.Add(LOG_TVIO_ERROR, "Write Error", ec.message());
        return false;
    } 
    return true;
}

bool TradingView::TVIO::Read(std::string& messageBuffer)
{
    beast::error_code ec;
    beast::flat_buffer buffer;
    m_Ws.read(buffer, ec);
    if (ec) {
        m_Log.Add(LOG_TVIO_ERROR, "Read Error", ec.message());
        return false;
    }
    messageBuffer = boost::beast::buffers_to_string(buffer.data());
    return true;
}