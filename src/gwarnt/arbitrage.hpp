// SPDX-License-Identifier: GPL-2.0-only

#ifndef GWARNT__ARBITRAGE_HPP
#define GWARNT__ARBITRAGE_HPP

#include <gwarnt/p2p_ad.hpp>

namespace gwarnt {

// Arbitrage opportunity
struct arb_opp {
	p2p_ad buy;
	p2p_ad sell;
	double est_profit;
	double max_possible_profit;
};

std::vector<struct arb_opp> find_arbitrage_opps(const std::vector<p2p_ad> &sell,
						const std::vector<p2p_ad> &buy,
						bool skip_dup_maker,
						double min_mpp,
						double min_price,
						double max_price);

} /* namespace gwarnt */

#endif /* GWARNT__ARBITRAGE_HPP */
