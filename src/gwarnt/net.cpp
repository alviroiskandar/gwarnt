// SPDX-License-Identifier: GPL-2.0-only

#include <gwarnt/net.hpp>

#include <exception>
#include <mutex>

namespace gwarnt {

static bool g_curl_initialized = false;
static std::mutex g_curl_mutex;

// static
void net::global_clean_up(void)
{
	std::lock_guard<std::mutex> lock(g_curl_mutex);

	if (g_curl_initialized) {
		curl_global_cleanup();
		g_curl_initialized = false;
	}
}

net::net(void)
{
	std::lock_guard<std::mutex> lock(g_curl_mutex);

	if (!g_curl_initialized) {
		if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK)
			throw std::runtime_error("curl_global_init() failed");

		g_curl_initialized = true;
	}

	ch_ = curl_easy_init();
	if (!ch_)
		throw std::runtime_error("curl_easy_init() failed");
}

net::~net(void)
{
	curl_easy_cleanup(ch_);
}

void net::set_method(const std::string &method)
{
	method_ = method;
}

void net::set_url(const std::string &url)
{
	url_ = url;
}

void net::set_data(const std::string &data)
{
	data_ = data;
}

void net::set_cookie(const std::string &cookie)
{
	cookie_ = cookie;
}

void net::set_user_agent(const std::string &user_agent)
{
	user_agent_ = user_agent;
}

void net::set_proxy(const std::string &proxy)
{
	proxy_ = proxy;
}

void net::set_headers(std::map<std::string, std::string> headers)
{
	headers_ = std::move(headers);
}

void net::set_curl_opt(CURLoption opt, const void *param)
{
	curl_easy_setopt(ch_, opt, param);
}

void net::add_header(const std::string &key, const std::string &value)
{
	headers_[key] = value;
}

void net::clear_headers(void)
{
	headers_.clear();
}

static size_t curl_write_callback(void *ptr, size_t size, size_t nmemb, void *data)
{
	std::string *resp = static_cast<std::string *>(data);

	try {
		resp->append(static_cast<char *>(ptr), size * nmemb);
	} catch (...) {
		return 0;
	}

	return size * nmemb;
}

void net::exec(void)
{
	struct curl_slist *slist = nullptr;
	CURLcode res;

	if (!ch_)
		throw std::runtime_error("Missing CURL handle");

	if (headers_.size() > 0) {
		for (const auto &h : headers_) {
			std::string header = h.first + ": " + h.second;
			slist = curl_slist_append(slist, header.c_str());
		}
		curl_easy_setopt(ch_, CURLOPT_HTTPHEADER, slist);
	}

	curl_easy_setopt(ch_, CURLOPT_URL, url_.c_str());
	curl_easy_setopt(ch_, CURLOPT_CUSTOMREQUEST, method_.c_str());
	curl_easy_setopt(ch_, CURLOPT_POSTFIELDS, data_.c_str());
	curl_easy_setopt(ch_, CURLOPT_COOKIE, cookie_.c_str());
	curl_easy_setopt(ch_, CURLOPT_USERAGENT, user_agent_.c_str());
	curl_easy_setopt(ch_, CURLOPT_PROXY, proxy_.c_str());
	curl_easy_setopt(ch_, CURLOPT_WRITEFUNCTION, &curl_write_callback);
	curl_easy_setopt(ch_, CURLOPT_WRITEDATA, &resp_);
	curl_easy_setopt(ch_, CURLOPT_TIMEOUT, 30L);
	curl_easy_setopt(ch_, CURLOPT_CONNECTTIMEOUT, 10L);

	resp_.clear();
	res = curl_easy_perform(ch_);
	curl_slist_free_all(slist);
	if (res != CURLE_OK) {
		std::string err = "Curl error: ";
		const char *cee;

		err += std::to_string(res) + ": ";
		cee = curl_easy_strerror(res);
		if (cee)
			err += cee;

		throw std::runtime_error(err);
	}
}

} /* namespace gwarnt */
