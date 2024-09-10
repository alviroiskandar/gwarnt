// SPDX-License-Identifier: GPL-2.0-only

#ifndef GWARNT__NET_HPP
#define GWARNT__NET_HPP

#include <curl/curl.h>

#include <iostream>
#include <cstdio>
#include <string>
#include <map>

namespace gwarnt {

class net {
public:

	net(void);
	~net(void);

	void set_method(const std::string &method);
	void set_url(const std::string &url);
	void set_data(const std::string &data);
	void set_cookie(const std::string &cookie);
	void set_user_agent(const std::string &user_agent);
	void set_proxy(const std::string &proxy);
	void set_headers(std::map<std::string, std::string> headers);
	void add_header(const std::string &key, const std::string &value);
	void set_curl_opt(CURLoption opt, const void *param);
	void clear_headers(void);
	void exec(void);

	inline const std::string &get_resp(void) const { return resp_; }

	static void global_clean_up(void);

private:
	CURL		*ch_ = nullptr;

	std::string	method_ = "GET";
	std::string	url_;
	std::string	data_;
	std::string	cookie_;
	std::string	user_agent_;
	std::string	proxy_;
	std::map<std::string, std::string> headers_;

	std::string	resp_;
};

} /* namespace gwarnt */

#endif /* #ifndef GWARNT__NET_HPP */
