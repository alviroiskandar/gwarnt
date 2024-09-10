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
	void broadcast_opp(const gwarnt::arb_opp &opp);
	void garbage_collect(void);

private:
	static constexpr time_t BROADCAST_INTERVAL = 300;
	gwarnt::tgbot tg_;
	std::string chat_id_;
	std::unordered_map<std::string, time_t> broadcast_map_;

	bool opp_should_skip(const gwarnt::arb_opp &opp);
	void insert_broadcast_map(const gwarnt::arb_opp &opp);
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
bool gwarnt_bot::opp_should_skip(const gwarnt::arb_opp &opp)
{
	std::string hash = get_opp_hash(opp);
	auto it = broadcast_map_.find(hash);

	if (it == broadcast_map_.end())
		return true;

	if (time(NULL) - it->second > BROADCAST_INTERVAL) {
		broadcast_map_.erase(it);
		return true;
	}

	return false;
}

inline
void gwarnt_bot::insert_broadcast_map(const gwarnt::arb_opp &opp)
{
	std::string hash = get_opp_hash(opp);
	broadcast_map_[hash] = time(NULL);
	printf("Inserted hash %s\n", hash.c_str());
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
void gwarnt_bot::broadcast_opp(const gwarnt::arb_opp &opp)
{
	if (!opp_should_skip(opp))
		return;

	const auto &b = opp.buy;
	const auto &s = opp.sell;
	double est_profit;
	double min_avail;
	double mpp;

	if (b.tradable_amount_ < s.tradable_amount_)
		min_avail = b.tradable_amount_;
	else
		min_avail = s.tradable_amount_;

	est_profit = b.price_ - s.price_;
	mpp = min_avail * est_profit;
	if (mpp < 100000)
		return;

	std::string msg = "Arbitrage opportunity detected!\n\n";

	msg += "Buy on " + s.exchange_ + "\n";
	msg += "m: " + s.ad_id_ + " (" + s.merchant_name_ + ")\n";
	msg += "price: " + to_string_wp(s.price_, 2) + " " + s.fiat_ + "\n";
	msg += "min_buy: " + to_string_wp(s.min_amount_, 2) + " " + s.fiat_ + "\n";
	msg += "max_buy: " + to_string_wp(s.max_amount_, 2) + " " + s.fiat_ + "\n";
	msg += "available: " + to_string_wp(s.tradable_amount_, 2) + " " + s.crypto_ + "\n";
	msg += "methods: " + merge_vec_str(s.methods_) + "\n";

	msg += "\n";

	msg += "Sell on " + b.exchange_ + "\n";
	msg += "m: " + b.ad_id_ + " (" + b.merchant_name_ + ")\n";
	msg += "price: " + to_string_wp(b.price_, 2) + " " + b.fiat_ + "\n";
	msg += "min_sell: " + to_string_wp(b.min_amount_, 2) + " " + b.fiat_ + "\n";
	msg += "max_sell: " + to_string_wp(b.max_amount_, 2) + " " + b.fiat_ + "\n";
	msg += "available: " + to_string_wp(b.tradable_amount_, 2) + " " + b.crypto_ + "\n";
	msg += "methods: " + merge_vec_str(b.methods_) + "\n";

	msg += "------------------------------------------\n";

	msg += "est_profit_per_unit: " + to_string_wp(est_profit, 2) + " " + s.fiat_ + "\n";
	msg += "max_possible_profit: " + to_string_wp(mpp, 2) + " " + s.fiat_ + "\n";

	printf("%s", msg.c_str());
	tg_.send_message(chat_id_, msg);
	insert_broadcast_map(opp);
	sleep(1);
}

static int broadcast_opp(gwarnt_bot *bot, const gwarnt::arb_opp &opp)
{
	bot->broadcast_opp(opp);
	return 0;
}

static int broadcast_opps(gwarnt_bot *bot,
			  const std::vector<gwarnt::arb_opp> &opps)
{
	std::string msg;
	gwarnt::tgbot::json j;

	if (opps.size() == 0)
		return 0;

	for (const auto &opp : opps)
		broadcast_opp(bot, opp);

	return 0;
}

static void run(gwarnt_bot *bot, gwarnt::p2p::binance *bnc,
		gwarnt::p2p::okx *okx)
{
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

	opps = gwarnt::find_arbitrage_opps(sell_okx, buy_bnc);
	printf("Opp1    : %zu\n", opps.size());
	broadcast_opps(bot, opps);

	opps = gwarnt::find_arbitrage_opps(sell_bnc, buy_okx);
	printf("Opp2    : %zu\n", opps.size());
	broadcast_opps(bot, opps);
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
