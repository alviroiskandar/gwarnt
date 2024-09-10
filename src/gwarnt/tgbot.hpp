// SPDX-License-Identifier: GPL-2.0-only

#ifndef GWARNT__TGBOT_HPP
#define GWARNT__TGBOT_HPP

#include <gwarnt/net.hpp>
#include <gwarnt/json.hpp>

#include <string>

namespace gwarnt {

typedef enum tgbot_send_msg {
	SEND_MSG_PLAIN,
	SEND_MSG_HTML,
	SEND_MSG_MARKDOWN,
} tgbot_send_msg_t;

class tgbot {
public:
	using json = nlohmann::json;

	tgbot(const std::string &token);
	tgbot(void);
	~tgbot(void);

	json send_message(const std::string &chat_id, const std::string &text,
			  tgbot_send_msg_t parse_mode = SEND_MSG_PLAIN);

	json get_me(void);

	void set_token(const std::string &token) { token_ = token; }

private:
	json exec_post(const std::string &method, const json &data);
	json exec_get(const std::string &method);
	void init(void);

	net net_;

	std::string token_;
};

} /* namespace gwarnt */

#endif /* #ifndef GWARNT__TGBOT_HPP */
