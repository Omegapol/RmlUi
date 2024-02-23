//
// Created by Mateusz Raczynski on 4/15/2021.
//

#include "DataFeed.h"

template<>
Rml::Vector<Rml::Vector2f> &Rml::DataFeedString<Rml::Vector2f>::Extract()
{
	auto vec = Vector<Vector2f>();
	auto el_list = split(_str, " ");
	for (auto &el: el_list) {
		auto val_list = split(el, ",");
		if (val_list.size() != 2)
			continue;
		auto x = std::stof(val_list[0]);
		auto y = std::stof(val_list[1]);
		vec.push_back({x, y});
	}
	cached = vec;
	return cached;
}