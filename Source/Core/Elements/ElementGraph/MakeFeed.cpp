//
// Created by Mateusz Raczynski on 7/20/2021.
//

#include "MakeFeed.h"
#include "DataFeedTypeCheck.h"
#include "DataGraphTypes.h"
#include "DataFeedTransforms.h"
#include "DataFeed.h"

namespace Rml {

	void *
	impl::MakeFeedImpl(DataVariable data, FamilyId target_id, const String &transform_str, DataFeedAggArguments args) {
		if (data) {
			void *ptr = nullptr;
			auto out = static_cast<FamilyId>(-1);
			SharedPtr<TypeCheckerBase> checker_used;
			for (auto &checker: GetGraphTypes()) {
				checker.second->GetId(out);
				ptr = checker.second->get_raw(data);
				if (ptr) {
					checker_used = checker.second;
					break;
				}
				ptr = nullptr;
			}

			if (ptr) {
				auto res = impl::TransformFeedProtImpl(ptr,
													   out,
													   target_id,
													   transform_str, args);
				if (!res) {
					//release resource under ptr
					checker_used->release_raw(ptr);
				}
				return res;
			}
		}
		return nullptr;


//		if (var) {
//			if (var.Child(String("h")) &&
//				var.Child(String("l")) &&
//				var.Child(String("en")) &&
//				var.Child(String("ex"))) {
//				auto _feed = MakeUnique<DataFeed<CandleStickData>>(data);
//				auto res = impl::TransformFeedProtImpl(_feed.get(),
//													   Family<CandleStickData>::Id(),
//													   target_id,
//													   transform_str, args);
//				if (res != nullptr)
//					_feed.release();
//				return res;
//			} else if (var.Child(String("x")) &&
//					   var.Child(String("y"))) {
//				auto _feed = MakeUnique<DataFeed<Vector2f>>(data);
//				auto res = impl::TransformFeedProtImpl(_feed.get(),
//													   Family<Vector2f>::Id(),
//													   target_id,
//													   transform_str, args);
//				if (res != nullptr)
//					_feed.release();
//				return res;
//			}
//		}
//		return nullptr;
	}
}