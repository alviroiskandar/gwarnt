// SPDX-License-Identifier: GPL-2.0-only

#include <gwarnt/tgbot.hpp>

using json = nlohmann::json;

namespace gwarnt {

tgbot::tgbot(const std::string &token):
	token_(token)
{
	init();
}

tgbot::tgbot(void)
{
	init();
}

inline void tgbot::init(void)
{
	net_.add_header("Content-Type", "application/json");
}

tgbot::~tgbot(void) = default;

json tgbot::send_message(const std::string &chat_id, const std::string &text,
			 tgbot_send_msg_t parse_mode)
{
	json j;

	j["chat_id"] = chat_id;
	j["text"] = text;

	switch (parse_mode) {
	case SEND_MSG_HTML:
		j["parse_mode"] = "HTML";
		break;
	case SEND_MSG_MARKDOWN:
		j["parse_mode"] = "Markdown";
		break;
	default:
		break;
	}

	return exec_post("sendMessage", j);
}

json tgbot::get_me(void)
{
	return exec_get("getMe");
}

json tgbot::exec_post(const std::string &method, const json &data)
{
	std::string url = "https://api.telegram.org/bot" + token_ + "/" + method;

	net_.set_url(url);
	net_.set_data(data.dump());
	net_.set_method("POST");
	net_.exec();
	return json::parse(net_.get_resp());
}

json tgbot::exec_get(const std::string &method)
{
	std::string url = "https://api.telegram.org/bot" + token_ + "/" + method;

	net_.set_url(url);
	net_.set_method("GET");
	net_.exec();
	return json::parse(net_.get_resp());
}

} /* namespace gwarnt */
