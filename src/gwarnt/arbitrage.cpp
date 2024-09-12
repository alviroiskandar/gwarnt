// SPDX-License-Identifier: GPL-2.0-only

#include <gwarnt/arbitrage.hpp>

namespace gwarnt {

std::vector<struct arb_opp> find_arbitrage_opps(const std::vector<p2p_ad> &sell,
						const std::vector<p2p_ad> &buy,
						double min_mpp,
						double min_price,
						double max_price)
{
	std::vector<struct arb_opp> ret;

	for (const auto &s: sell) {
		for (const auto &b: buy) {
			if (s.price_ < b.price_) {
				struct arb_opp opp;
				double est_profit;
				double min_avail;
				double mpp;

				if (b.tradable_amount_ < s.tradable_amount_)
					min_avail = b.tradable_amount_;
				else
					min_avail = s.tradable_amount_;

				est_profit = b.price_ - s.price_;
				mpp = min_avail * est_profit;
				if (min_mpp != 0.0 && mpp < min_mpp)
					continue;
				if (min_price != 0.0 && s.price_ < min_price)
					continue;
				if (max_price != 0.0 && b.price_ > max_price)
					continue;

				opp.buy = b;
				opp.sell = s;
				opp.est_profit = est_profit;
				opp.max_possible_profit = mpp;
				ret.push_back(opp);
			}
		}
	}

	return ret;
}

} /* namespace gwarnt */
