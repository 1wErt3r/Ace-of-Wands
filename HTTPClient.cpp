#include "HTTPClient.h"
#include <iostream>


HTTPClient::HTTPClient()
	:
	mIOContext(std::make_unique<net::io_context>()),
	mSSLContext(std::make_unique<ssl::context>(ssl::context::tlsv12_client))
{
	mSSLContext->set_default_verify_paths();
	mSSLContext->set_verify_mode(ssl::verify_peer);
}

HTTPClient::~HTTPClient() = default;


BString
HTTPClient::Post(const BString& url, const BString& jsonData, const BString& authHeader)
{
	try {
		BString host = ExtractHostFromURL(url);
		BString target = ExtractTargetFromURL(url);

		return PerformHTTPSRequest(host, target, jsonData, authHeader);
	} catch (const std::exception& e) {
		BString errorMsg = "HTTP Request Error: ";
		errorMsg += e.what();
		return errorMsg;
	}
}


BString
HTTPClient::PerformHTTPSRequest(const BString& host, const BString& target, const BString& jsonData,
	const BString& authHeader)
{
	try {
		// Resolve the hostname
		tcp::resolver resolver(*mIOContext);
		auto const results = resolver.resolve(host.String(), "443");

		// Create SSL stream
		ssl::stream<tcp::socket> stream(*mIOContext, *mSSLContext);

		// Set SNI Hostname
		if (!SSL_set_tlsext_host_name(stream.native_handle(), host.String())) {
			beast::error_code ec{static_cast<int>(::ERR_get_error()),
				net::error::get_ssl_category()};
			throw beast::system_error{ec};
		}

		// Connect to the server
		net::connect(stream.next_layer(), results.begin(), results.end());

		// Perform SSL handshake
		stream.handshake(ssl::stream_base::client);

		// Set up HTTP POST request
		http::request<http::string_body> req{http::verb::post, target.String(), 11};
		req.set(http::field::host, host.String());
		req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
		req.set(http::field::content_type, "application/json");
		req.set(http::field::authorization, authHeader.String());
		req.set(http::field::accept, "application/json");
		req.body() = jsonData.String();
		req.prepare_payload();

		// Send the HTTP request
		http::write(stream, req);

		// Receive the HTTP response
		beast::flat_buffer buffer;
		http::response<http::string_body> res;
		http::read(stream, buffer, res);


		beast::error_code ec;
		stream.shutdown(ec);

		return BString(res.body().c_str());
	} catch (const std::exception& e) {
		std::cout << "HTTPS request failed: " << e.what() << std::endl;
		throw;
	}
}


BString
HTTPClient::ExtractHostFromURL(const BString& url)
{
	std::string urlStr(url.String());
	size_t start = urlStr.find("://");
	if (start == std::string::npos)
		throw std::runtime_error("Invalid URL format");

	start += 3; // Skip past "://"
	size_t end = urlStr.find('/', start);

	if (end == std::string::npos)
		return BString(urlStr.substr(start).c_str());

	return BString(urlStr.substr(start, end - start).c_str());
}


BString
HTTPClient::ExtractTargetFromURL(const BString& url)
{
	std::string urlStr(url.String());
	size_t start = urlStr.find("://");
	if (start == std::string::npos)
		throw std::runtime_error("Invalid URL format");

	start += 3; // Skip past "://"
	size_t slashPos = urlStr.find('/', start);

	if (slashPos == std::string::npos)
		return BString("/");

	return BString(urlStr.substr(slashPos).c_str());
}
