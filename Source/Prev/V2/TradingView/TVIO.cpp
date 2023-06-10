#include "TVIO.h"

TradingView::TVIO::TVIO(void) :
	m_Ioc(),
    m_Work(m_Ioc),
	m_Ctx{ ssl::context::tlsv12_client },
	m_Resolver{ m_Ioc },
    m_Ws{ m_Ioc, m_Ctx },
	m_Host("prodata.tradingview.com"),
	m_Port("443")
{
}

TradingView::TVIO::~TVIO(void)
{
}

bool TradingView::TVIO::Connect(void)
{
    auto const results = m_Resolver.resolve(m_Host, m_Port);

    net::connect(m_Ws.next_layer().next_layer(), results.begin(), results.end());

    boost::asio::ip::address clientAddr = m_Ws.next_layer().next_layer().local_endpoint().address();
    boost::asio::ip::address serverAddr = m_Ws.next_layer().next_layer().remote_endpoint().address();

    std::cout << "ClientAddr: " << clientAddr << std::endl;
    std::cout << "ServerAddr: " << serverAddr << std::endl << std::endl;

    SSL_set_tlsext_host_name(m_Ws.next_layer().native_handle(), m_Host.c_str());
    m_Ws.next_layer().handshake(ssl::stream_base::client);

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
    m_Ws.handshake(response, m_Host, "/socket.io/websocket");

    std::cout << response.base() << std::endl << std::endl;
    std::cout << response.body() << std::endl;
    return true;
}

bool TradingView::TVIO::Disconnect(void)
{
    beast::error_code closeError;
    m_Ws.close(websocket::close_code::normal, closeError);
    if (closeError) std::cout << "CloseError: " << closeError.message() << std::endl;
    return true;
}

bool TradingView::TVIO::Write(std::string message)
{
    beast::error_code ec;
    m_Ws.write(net::buffer(message), ec);
    if (ec) std::cout << ec.message() << std::endl;
    return true;
}

std::string TradingView::TVIO::Read(void)
{
    beast::error_code ec;
    beast::flat_buffer buffer;
    m_Ws.read(buffer, ec);

    if (ec) {
        std::cout << ec.message() << std::endl;
        return ec.message();
    }

    return boost::beast::buffers_to_string(buffer.data());
}

void TradingView::TVIO::AsyncRead(boost::system::error_code ec, std::size_t bytes)
{
    beast::flat_buffer buffer;
    m_Ws.async_read(buffer, std::bind(&TradingView::TVIO::AsyncRead, this, std::placeholders::_1, std::placeholders::_2));

    m_AsyncReadCallback(boost::beast::buffers_to_string(buffer.data()));
}
