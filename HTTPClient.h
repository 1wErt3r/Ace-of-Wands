#pragma once

#include <String.h>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <memory>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = net::ssl;
using tcp = net::ip::tcp;

class HTTPClient {
public:
	HTTPClient();
	~HTTPClient();

	BString Post(const BString& url, const BString& jsonData, const BString& authHeader);

private:
	std::unique_ptr<net::io_context> mIOContext;
	std::unique_ptr<ssl::context> mSSLContext;

	BString PerformHTTPSRequest(const BString& host, const BString& target, const BString& jsonData,
		const BString& authHeader);

	static BString ExtractHostFromURL(const BString& url);
	static BString ExtractTargetFromURL(const BString& url);
};
