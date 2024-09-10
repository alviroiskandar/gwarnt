// SPDX-License-Identifier: GPL-2.0-only

#include <gwarnt/arbitrage.hpp>

namespace gwarnt {

std::vector<struct arb_opp> find_arbitrage_opps(const std::vector<p2p_ad> &sell,
						const std::vector<p2p_ad> &buy)
{
	std::vector<struct arb_opp> ret;

	for (const auto &s: sell) {
		for (const auto &b: buy) {
			if (s.price_ < b.price_) {
				struct arb_opp opp;

				opp.buy = b;
				opp.sell = s;
				ret.push_back(opp);
			}
		}
	}

	return ret;
}

} /* namespace gwarnt */
