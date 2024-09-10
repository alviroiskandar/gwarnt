// SPDX-License-Identifier: GPL-2.0-only

#include <unistd.h>
#include <iostream>
#include <gwarnt/p2p/okx.hpp>
#include <gwarnt/p2p/binance.hpp>
#include <gwarnt/arbitrage.hpp>

int main(void)
{
	std::vector<gwarnt::p2p_ad> buy_bnc, sell_bnc;
	std::vector<gwarnt::p2p_ad> buy_okx, sell_okx;
	std::vector<gwarnt::arb_opp> opportunities;
	gwarnt::p2p::binance bnc;
	gwarnt::p2p::okx okx;

	while (1) {
		printf("Fetching data ...\n");
		buy_bnc = bnc.get_data("IDR", "USDT", "BUY");
		sell_bnc = bnc.get_data("IDR", "USDT", "SELL");

		buy_okx = okx.get_data("IDR", "USDT", "BUY");
		sell_okx = okx.get_data("IDR", "USDT", "SELL");
		printf("Binance: %lu buy, %lu sell\n", buy_bnc.size(), sell_bnc.size());
		printf("OKX: %lu buy, %lu sell\n", buy_okx.size(), sell_okx.size());

		opportunities = gwarnt::find_arbitrage_opps(sell_okx, buy_bnc);
		for (const auto &i : opportunities) {
			std::cout << i.sell.dump() << std::endl;
			std::cout << "====================" << std::endl;
			std::cout << i.buy.dump() << std::endl;
			std::cout << "----------------------------------------------" << std::endl;
		}

		opportunities = gwarnt::find_arbitrage_opps(sell_bnc, buy_okx);
		for (const auto &i : opportunities) {
			std::cout << i.sell.dump() << std::endl;
			std::cout << "====================" << std::endl;
			std::cout << i.buy.dump() << std::endl;
			std::cout << "----------------------------------------------" << std::endl;
		}

		printf("Sleeping...\n");
		sleep(5);
	}

	return 0;
}
