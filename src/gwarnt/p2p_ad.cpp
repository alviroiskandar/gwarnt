// SPDX-License-Identifier: GPL-2.0-only

#include <gwarnt/p2p_ad.hpp>
#include <cstddef>

namespace gwarnt {

std::string p2p_ad::dump(void) const
{
	std::string ret = "";
	size_t i = 0;

	ret += "ad_id: " + ad_id_ + "\n";
	ret += "merchant_name: " + merchant_name_ + "\n";
	ret += "fiat: " + fiat_ + "\n";
	ret += "crypto: " + crypto_ + "\n";
	ret += "trade_type: " + trade_type_ + "\n";
	ret += "exchange: " + exchange_ + "\n";
	ret += "price: " + std::to_string(price_) + "\n";
	ret += "min_amount: " + std::to_string(min_amount_) + "\n";
	ret += "max_amount: " + std::to_string(max_amount_) + "\n";
	ret += "tradable_amount: " + std::to_string(tradable_amount_) + "\n";

	ret += "methods: ";
	for (const auto &m : methods_)
		ret += (i++ > 0 ? ", " : "") + m;

	return ret;
}

} /* namespace gwarnt */
