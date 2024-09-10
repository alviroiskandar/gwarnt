// SPDX-License-Identifier: GPL-2.0-only

#include <gwarnt/p2p/okx.hpp>
#include <gwarnt/helpers.hpp>
#include <gwarnt/json.hpp>
#include <exception>

using json = nlohmann::json;

namespace gwarnt {
namespace p2p {

okx::okx(void)
{
	net_.add_header("Content-Type", "application/json");
}

okx::~okx(void) = default;

void okx::set_fiat(const std::string &fiat)
{
	fiat_ = fiat;
}

void okx::set_crypto(const std::string &crypto)
{
	crypto_ = crypto;
}

void okx::set_trade_type(const std::string &trade_type)
{
	trade_type_ = trade_type;
}

std::vector<gwarnt::p2p_ad> okx::get_data(void)
{
	return get_data(fiat_, crypto_, trade_type_);
}

std::vector<gwarnt::p2p_ad> okx::get_data(const std::string &fiat,
					  const std::string &crypto,
					  const std::string &trade_type)
{
	std::vector<gwarnt::p2p_ad> ads;
	std::string type;
	std::string url;
	std::string p;
	json *jent;
	json j;

	type = trade_type;
	strtoupper(type);
	if (type != "BUY" && type != "SELL")
		throw std::runtime_error("Invalid trade type: " + trade_type);

	p = "side=" + trade_type + "&fiatCurrency=" + fiat + "&cryptoCurrency=" + crypto;
	url = "https://www.okx.com/v3/c2c/tradingOrders/getMarketplaceAdsPrelogin?&paymentMethod=all&userType=all&hideOverseasVerificationAds=false&sortType=price_asc&limit=1000&currentPage=1&numberPerPage=1000&"
		+ p;

	net_.set_method("GET");
	net_.set_url(url);
	net_.set_curl_opt(CURLOPT_ACCEPT_ENCODING, "gzip");
	net_.exec();

	const std::string &res = net_.get_resp();
	j = json::parse(res);

	if (type == "SELL")
		jent = &j["data"]["sell"];
	else
		jent = &j["data"]["buy"];

	for (const auto &q : *jent) {
		try {
			gwarnt::p2p_ad p;

			p.ad_id_ = q["id"];
			p.merchant_name_ = q["nickName"];
			p.trade_type_ = type;
			p.fiat_ = fiat;
			p.crypto_ = crypto;
			p.price_ = strtod(q["price"].get<std::string>().c_str(), nullptr);
			p.min_amount_ = strtod(q["quoteMinAmountPerOrder"].get<std::string>().c_str(), nullptr);
			p.max_amount_ = strtod(q["quoteMaxAmountPerOrder"].get<std::string>().c_str(), nullptr);
			p.tradable_amount_ = strtod(q["availableAmount"].get<std::string>().c_str(), nullptr);
			p.exchange_ = "okx";

			for (auto &r : q["paymentMethods"])
				p.methods_.push_back(r);

			ads.push_back(p);
		} catch (const std::exception &e) {
			printf("gwarnt::p2p::okx::get_data: %s\n", e.what());
		}
	}

	return ads;
}

} /* namespace gwarnt */
} /* namespace p2p */
