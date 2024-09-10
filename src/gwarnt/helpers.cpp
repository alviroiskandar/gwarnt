// SPDX-License-Identifier: GPL-2.0-only

#include <gwarnt/helpers.hpp>

#include <cctype>

namespace gwarnt {

void strtolower(std::string &str)
{
	for (auto &i : str)
		i = std::tolower(i);
}

void strtoupper(std::string &str)
{
	for (auto &i : str)
		i = std::toupper(i);
}

} /* namespace gwarnt */
