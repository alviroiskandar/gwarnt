// SPDX-License-Identifier: GPL-2.0-only

#include <gwarnt/arbitrage.hpp>

namespace gwarnt {

std::vector<struct arb_opp> find_arbitrage_opps(const std::vector<p2p_ad> &sell,
						const std::vector<p2p_ad> &buy)
{
	std::vector<struct arb_opp> ret;

	for (const auto &i : buy) {
		for (const auto &j : sell) {
			if (j.price_ < i.price_) {
				struct arb_opp opp;

				opp.buy = i;
				opp.sell = j;
				ret.push_back(opp);
			}
		}
	}

	return ret;
}

} /* namespace gwarnt */
