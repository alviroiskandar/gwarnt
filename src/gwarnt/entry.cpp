// SPDX-License-Identifier: GPL-2.0-only

#include <unistd.h>
#include <ctime>
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <unordered_map>

#include <gwarnt/tgbot.hpp>
#include <gwarnt/p2p/okx.hpp>
#include <gwarnt/p2p/binance.hpp>
#include <gwarnt/arbitrage.hpp>
#include <gwarnt/helpers.hpp>

template <typename T>
std::string to_string_wp(const T a_value, const int n = 2)
{
	std::ostringstream out;
	out.precision(n);
	out << std::fixed << a_value;
	return std::move(out).str();
}

class gwarnt_bot {
public:
	inline void set_token(const std::string &token) { tg_.set_token(token); }
	inline void set_chat_id(const std::string &chat_id) { chat_id_ = chat_id; }

	void init_get_me(void);
	void broadcast_opps(const std::vector<gwarnt::arb_opp> &opps);
	void garbage_collect(void);

private:
	static constexpr time_t BROADCAST_INTERVAL = 300;
	gwarnt::tgbot tg_;
	std::string chat_id_;
	std::unordered_map<std::string, time_t> broadcast_map_;

	bool opp_should_skip(const gwarnt::arb_opp &opp);
	bool opp_should_skip(const std::string &hash);
	void insert_broadcast_map(const gwarnt::arb_opp &opp);
	void insert_broadcast_map(const std::string &hash);
	std::string get_opp_hash(const gwarnt::arb_opp &opp);
};

inline
void gwarnt_bot::init_get_me(void)
{
	auto j = tg_.get_me();
	auto username = j["result"]["username"].get<std::string>();

	printf("========================================\n");
	printf("Bot username   : %s\n", username.c_str());
	printf("Target Chat ID : %s\n", chat_id_.c_str());
	printf("========================================\n");
}

inline
std::string gwarnt_bot::get_opp_hash(const gwarnt::arb_opp &opp)
{
	std::string hash = "";

	hash += opp.buy.ad_id_;
	hash += opp.sell.ad_id_;
	hash += to_string_wp(opp.buy.price_);
	hash += to_string_wp(opp.sell.price_);

	return hash;
}

inline
void gwarnt_bot::garbage_collect(void)
{
	auto it = broadcast_map_.begin();

	while (it != broadcast_map_.end()) {
		if (time(NULL) - it->second > BROADCAST_INTERVAL)
			it = broadcast_map_.erase(it);
		else
			++it;
	}
}

inline
bool gwarnt_bot::opp_should_skip(const std::string &hash)
{
	auto it = broadcast_map_.find(hash);

	if (it == broadcast_map_.end())
		return false;

	if (time(NULL) - it->second > BROADCAST_INTERVAL) {
		broadcast_map_.erase(it);
		return false;
	}

	return true;
}

inline
bool gwarnt_bot::opp_should_skip(const gwarnt::arb_opp &opp)
{
	std::string hash = get_opp_hash(opp);
	return opp_should_skip(hash);
}

inline
void gwarnt_bot::insert_broadcast_map(const std::string &hash)
{
	broadcast_map_[hash] = time(NULL);
	printf("Inserted hash %s\n", hash.c_str());
}

inline
void gwarnt_bot::insert_broadcast_map(const gwarnt::arb_opp &opp)
{
	std::string hash = get_opp_hash(opp);
	insert_broadcast_map(hash);
}

static std::string merge_vec_str(const std::vector<std::string> &vec)
{
	std::string ret = "";
	size_t i = 0;

	for (const auto &s : vec)
		ret += (++i > 1 ? ", " : "") + s;

	return ret;
}

inline
void gwarnt_bot::broadcast_opps(const std::vector<gwarnt::arb_opp> &opps)
{
	std::string msg = "#p2p_arbitrage\nArbitrage opportunity detected!\n\n";
	std::vector<std::string> hashes;
	bool send_after_loop = false;
	std::string f = "";

	for (const auto &opp : opps) {
		std::string hash = get_opp_hash(opp);
		if (opp_should_skip(hash))
			continue;

		hashes.push_back(hash);
		const auto &s = opp.sell;
		const auto &b = opp.buy;
		size_t len;

		f += "Buy on " + s.exchange_ + "\n";
		f += "m: " + s.ad_id_ + " (" + s.merchant_name_ + ")\n";
		f += "price: " + to_string_wp(s.price_, 2) + " " + s.fiat_ + "\n";
		f += "min_buy: " + to_string_wp(s.min_amount_, 2) + " " + s.fiat_ + "\n";
		f += "max_buy: " + to_string_wp(s.max_amount_, 2) + " " + s.fiat_ + "\n";
		f += "available: " + to_string_wp(s.tradable_amount_, 2) + " " + s.crypto_ + "\n";
		f += "mt: " + merge_vec_str(s.methods_) + "\n";
		f += "\n";
		f += "Sell on " + b.exchange_ + "\n";
		f += "m: " + b.ad_id_ + " (" + b.merchant_name_ + ")\n";
		f += "price: " + to_string_wp(b.price_, 2) + " " + b.fiat_ + "\n";
		f += "min_sell: " + to_string_wp(b.min_amount_, 2) + " " + b.fiat_ + "\n";
		f += "max_sell: " + to_string_wp(b.max_amount_, 2) + " " + b.fiat_ + "\n";
		f += "available: " + to_string_wp(b.tradable_amount_, 2) + " " + b.crypto_ + "\n";
		f += "mt: " + merge_vec_str(b.methods_) + "\n";
		f += "------------------------------------------\n";
		f += "est_profit_per_unit: " + to_string_wp(opp.est_profit, 2) + " " + s.fiat_ + "\n";
		f += "max_possible_profit: " + to_string_wp(opp.max_possible_profit, 2) + " " + s.fiat_ + "\n";
		f += "============================================\n\n";

		len = msg.length() + f.length();
		if (len >= 4096) {
			tg_.send_message(chat_id_, msg);

			for (const auto &hash : hashes)
				insert_broadcast_map(hash);

			hashes.clear();
			msg = "#p2p_arbitrage\nArbitrage opportunity detected!\n\n" + f;
			f = "";
			sleep(2);

			send_after_loop = false;
		} else {
			send_after_loop = true;
		}

		msg += f;
	}

	if (send_after_loop) {
		msg += f;
		tg_.send_message(chat_id_, msg);
		for (const auto &hash : hashes)
			insert_broadcast_map(hash);

		sleep(1);
	}
}

static void run(gwarnt_bot *bot, gwarnt::p2p::binance *bnc,
		gwarnt::p2p::okx *okx)
{
	static const double min_profit = 100000;
	static const double min_price = 14000;
	static const double max_price = 17000;

	std::vector<gwarnt::p2p_ad> buy_bnc, sell_bnc;
	std::vector<gwarnt::p2p_ad> buy_okx, sell_okx;
	std::vector<gwarnt::arb_opp> opps;

	printf("Fetching data...\n");

	buy_bnc = bnc->get_data("IDR", "USDT", "BUY");
	sell_bnc = bnc->get_data("IDR", "USDT", "SELL");
	buy_okx = okx->get_data("IDR", "USDT", "BUY");
	sell_okx = okx->get_data("IDR", "USDT", "SELL");

	printf("Binance : %lu buy, %lu sell\n", buy_bnc.size(), sell_bnc.size());
	printf("OKX     : %lu buy, %lu sell\n", buy_okx.size(), sell_okx.size());

	opps = gwarnt::find_arbitrage_opps(sell_okx, buy_bnc, min_profit, min_price, max_price);
	printf("Opp1    : %zu\n", opps.size());
	bot->broadcast_opps(opps);

	opps = gwarnt::find_arbitrage_opps(sell_bnc, buy_okx, min_profit, min_price, max_price);
	printf("Opp2    : %zu\n", opps.size());
	bot->broadcast_opps(opps);
}

int main(void)
{
	gwarnt::p2p::binance bnc;
	gwarnt::p2p::okx okx;
	const char *tmp;
	gwarnt_bot bot;
	unsigned u = 5;
	
	tmp = std::getenv("TG_BOT_TOKEN");
	if (!tmp) {
		printf("Please set TG_BOT_TOKEN environment variable!\n");
		return 1;
	}
	bot.set_token(tmp);

	tmp = std::getenv("TG_CHAT_ID");
	if (!tmp) {
		printf("Please set TG_CHAT_ID environment variable!\n");
		return 1;
	}
	bot.set_chat_id(tmp);
	bot.init_get_me();

	while (1) {

		try {
			run(&bot, &bnc, &okx);
			u = 5;
		} catch (std::exception &e) {
			printf("Exception: %s\n", e.what());

			// Double the sleep time if exception occurs.
			u *= 2;
		}

		bot.garbage_collect();
		printf("Sleeping for %u seconds...\n", u);
		sleep(u);
	}

	return 0;
}
