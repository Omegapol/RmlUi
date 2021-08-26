//
// Created by Mateusz Raczynski on 7/20/2021.
//

#ifndef RMLUI_MAKEFEED_H
#define RMLUI_MAKEFEED_H

#include <RmlUi/Core/Elements/DataFeedBase.h>

namespace Rml {
	struct DataFeedAggArguments {
		double agg_size = 10;
	};
	namespace impl {
		void *
		MakeFeedImpl(DataVariable var, FamilyId target_id, const String &transform_str, DataFeedAggArguments args);
	}

	template<typename TargetType>
	UniquePtr <DataFeedBase<TargetType>>
	MakeFeed(DataVariable var, const String &transform_str, DataFeedAggArguments args) {
		void *res = impl::MakeFeedImpl(var, Family<TargetType>::Id(), transform_str, args);
		if (res) {
			auto *ptr = reinterpret_cast<DataFeedBase<TargetType> *>(res);
			auto ptr_res = UniquePtr<DataFeedBase<TargetType>>(ptr);
			return ptr_res;
		}
		return UniquePtr<DataFeedBase<TargetType>>();
	}
}
#endif //RMLUI_MAKEFEED_H
