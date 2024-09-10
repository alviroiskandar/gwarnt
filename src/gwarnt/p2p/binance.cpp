// SPDX-License-Identifier: GPL-2.0-only

#include <gwarnt/p2p/binance.hpp>
#include <gwarnt/helpers.hpp>
#include <gwarnt/json.hpp>
#include <exception>

using json = nlohmann::json;

namespace gwarnt {
namespace p2p {

binance::binance(void)
{
	net_.add_header("Content-Type", "application/json");
	net_.add_header("x-passthrough-token", "");
}

binance::~binance(void) = default;

void binance::set_fiat(const std::string &fiat)
{
	fiat_ = fiat;
}

void binance::set_crypto(const std::string &crypto)
{
	crypto_ = crypto;
}

void binance::set_trade_type(const std::string &trade_type)
{
	trade_type_ = trade_type;
}

std::vector<gwarnt::p2p_ad> binance::get_data(void)
{
	return get_data(fiat_, crypto_, trade_type_);
}

std::vector<gwarnt::p2p_ad> binance::get_data(const std::string &fiat,
					      const std::string &crypto,
					      const std::string &trade_type,
					      uint64_t page)
{
	std::vector<gwarnt::p2p_ad> ads;

	if (page == ~0ull) {
		std::vector<gwarnt::p2p_ad> tmp;
		size_t i;

		for (i = 1; i <= 5; i++) {
			tmp = __get_data(fiat, crypto, trade_type, i);
			ads.insert(ads.end(), tmp.begin(), tmp.end());
		}
	} else {
		ads = __get_data(fiat, crypto, trade_type, page);
	}

	return ads;
}

std::vector<gwarnt::p2p_ad> binance::__get_data(const std::string &fiat,
						const std::string &crypto,
						const std::string &trade_type,
						uint64_t page)
{
	std::vector<gwarnt::p2p_ad> ads;
	std::string type;
	json j;

	type = trade_type;
	strtoupper(type);
	if (type == "BUY")
		type = "SELL";
	else if (type == "SELL")
		type = "BUY";
	else
		throw std::runtime_error("Invalid trade type: " + trade_type);

	j["fiat"] = fiat;
	j["page"] = page;
	j["rows"] = 20;
	j["tradeType"] = type;
	j["asset"] = crypto;
	j["countries"] = json::array();
	j["proMerchantAds"] = false;
	j["shieldMerchantAds"] = false;
	j["filterType"] = "all";
	j["periods"] = json::array();
	j["additionalKycVerifyFilter"] = 0;
	j["publisherType"] = "merchant";
	j["payTypes"] = json::array();
	j["classifies"] = json::array();
	j["classifies"].push_back("mass");
	j["classifies"].push_back("profession");
	j["classifies"].push_back("fiat_trade");

	net_.set_url("https://p2p.binance.com/bapi/c2c/v2/friendly/c2c/adv/search");
	net_.set_method("POST");
	net_.set_data(j.dump());
	net_.set_curl_opt(CURLOPT_ACCEPT_ENCODING, "gzip");
	net_.exec();

	const std::string &res = net_.get_resp();
	j = json::parse(res);

	for (const auto &i : j["data"]) {
		try {
			const auto &adv = i["adv"];
			gwarnt::p2p_ad p;

			p.ad_id_ = adv["advNo"];
			p.merchant_name_ = i["advertiser"]["nickName"];
			p.fiat_ = adv["fiatUnit"];
			p.crypto_ = adv["asset"];
			p.trade_type_ = adv["tradeType"];
			p.price_ = strtod(adv["price"].get<std::string>().c_str(), nullptr);
			p.min_amount_ = strtod(adv["minSingleTransAmount"].get<std::string>().c_str(), nullptr);
			p.max_amount_ = strtod(adv["maxSingleTransAmount"].get<std::string>().c_str(), nullptr);
			p.tradable_amount_ = strtod(adv["tradableQuantity"].get<std::string>().c_str(), nullptr);
			p.exchange_ = "binance";

			for (auto &j : adv["tradeMethods"]) {
				try {
					p.methods_.push_back(j["tradeMethodName"]);
				} catch (...) {
					p.methods_.push_back("p:"+j["identifier"].get<std::string>());
				}
			}

			ads.push_back(p);
		} catch (std::exception &e) {
			printf("gwarnt::p2p::binance::__get_data: %s\n", e.what());
		}
	}

	return ads;
}

} /* namespace gwarnt */
} /* namespace p2p */
