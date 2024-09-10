// SPDX-License-Identifier: GPL-2.0-only

#ifndef GWARNT__ARBITRAGE_HPP
#define GWARNT__ARBITRAGE_HPP

#include <gwarnt/p2p_ad.hpp>

namespace gwarnt {

// Arbitrage opportunity
struct arb_opp {
	p2p_ad buy;
	p2p_ad sell;
};

std::vector<struct arb_opp> find_arbitrage_opps(const std::vector<p2p_ad> &sell,
						const std::vector<p2p_ad> &buy);

} /* namespace gwarnt */

#endif /* GWARNT__ARBITRAGE_HPP */
