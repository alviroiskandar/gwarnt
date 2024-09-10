// SPDX-License-Identifier: GPL-2.0-only

#ifndef GWARNT__P2PDS_HPP
#define GWARNT__P2PDS_HPP

#include <vector>
#include <string>

namespace gwarnt {

struct p2p_ad {
	std::string	ad_id_;
	std::string	merchant_name_;
	std::string	fiat_;
	std::string	crypto_;
	std::string	trade_type_;
	std::string	exchange_;

	double	price_;
	double	min_amount_;
	double	max_amount_;
	double	tradable_amount_;

	std::vector<std::string>	methods_;

	std::string dump(void) const;
};

} /* namespace gwarnt */

#endif /* #ifndef GWARNT__P2PDS_HPP */
