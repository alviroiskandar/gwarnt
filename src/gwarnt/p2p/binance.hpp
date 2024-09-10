// SPDX-License-Identifier: GPL-2.0-only

#ifndef GWARNT__P2P__BINANCE_HPP
#define GWARNT__P2P__BINANCE_HPP

#include <gwarnt/net.hpp>
#include <gwarnt/p2p_ad.hpp>

namespace gwarnt {
namespace p2p {

class binance {
public:
	binance(void);
	~binance(void);

	void set_fiat(const std::string &fiat);
	void set_crypto(const std::string &crypto);
	void set_trade_type(const std::string &trade_type);
	std::vector<gwarnt::p2p_ad> get_data(void);
	std::vector<gwarnt::p2p_ad> get_data(const std::string &fiat,
					     const std::string &crypto,
					     const std::string &trade_type,
					     uint64_t page = ~0ull);

private:
	std::vector<gwarnt::p2p_ad> __get_data(const std::string &fiat,
					      const std::string &crypto,
					      const std::string &trade_type,
					      uint64_t page);


	gwarnt::net	net_;
	std::string	fiat_;
	std::string	crypto_;
	std::string	trade_type_;
};

} /* namespace p2p */
} /* namespace gwarnt */

#endif /* GWARNT__P2P__BINANCE_HPP */
