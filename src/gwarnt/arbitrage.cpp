// SPDX-License-Identifier: GPL-2.0-only

#include <gwarnt/arbitrage.hpp>
#include <unordered_map>

namespace gwarnt {

std::vector<struct arb_opp> find_arbitrage_opps(const std::vector<p2p_ad> &sell,
						const std::vector<p2p_ad> &buy,
						bool skip_dup_maker,
						double min_mpp,
						double min_price,
						double max_price)
{
	std::unordered_map<std::string, bool> eli_makers;
	std::vector<struct arb_opp> ret;

	for (const auto &s: sell) {
		for (const auto &b: buy) {
			struct arb_opp opp;
			double est_profit;
			double min_avail;
			double mpp;

			if (b.price_ <= s.price_)
				continue;

			if (b.min_amount_ > s.max_amount_) {
				/*
				 * We can buy, but we can't sell scenario.
				 *
				 * Example:
				 *  - We buy from seller with max_amount 100
				 *  - We sell to buyer with min_amount 50
				 */
				continue;
			}

			if (s.min_amount_ > b.max_amount_) {
				/*
				 * We can sell, but we can't buy scenario.
				 *
				 * Example:
				 *  - We sell to buyer with max_amount 100
				 *  - We buy from seller with min_amount 50
				 */
				continue;
			}

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

			if (skip_dup_maker) {
				if (eli_makers.find(s.ad_id_) != eli_makers.end())
					continue;
				if (eli_makers.find(b.ad_id_) != eli_makers.end())
					continue;
				eli_makers[s.ad_id_] = true;
				eli_makers[b.ad_id_] = true;
			}

			opp.buy = b;
			opp.sell = s;
			opp.est_profit = est_profit;
			opp.max_possible_profit = mpp;
			ret.push_back(opp);
		}
	}

	return ret;
}

} /* namespace gwarnt */
