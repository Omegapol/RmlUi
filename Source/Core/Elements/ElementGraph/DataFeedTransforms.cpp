//
// Created by Mateusz Raczynski on 7/14/2021.
//

#include "DataFeedTransforms.h"
#include "DataFeedAggregate.h"
#include "DataFeedAggregateStep.h"
#include "DataFeedTypeCheck.h"

namespace Rml {
	void RegisterFactory(impl::FactoryStorageType & storage, SharedPtr < impl::FeedTransformFactoryBase > factory) {
		FamilyId in, out;
		factory->GetId(in, out);
		storage[in][out][factory->GetName()] = factory;
	}

	impl::FactoryStorageType get_factories() {
		impl::FactoryStorageType res;

		RegisterFactory(res,
						MakeShared<GenericAggFactory<
								DataFeedAggVector2Candles,
								Vector2f,
								CandleStickData>>("candle(x)"));
		RegisterFactory(res,
						MakeShared<GenericAggFactory<
								DataFeedAggStepVector2Candles,
								Vector2f,
								CandleStickData>>("candle_s(x)"));
		RegisterFactory(res,
						MakeShared<GenericAggFactory<
								DataFeedAggSum,
								Vector2f,
								Vector2f>>("sum(x)"));
		RegisterFactory(res,
						MakeShared<GenericAggFactory<
								DataFeedAggVecAvg,
								Vector2f,
								Vector2f>>("avg(x)"));
		return res;
	}

	void *
	impl::TransformFeedProtImpl(void *opaque_feed, FamilyId input_type, FamilyId target_output_type,
								const String &transform_str,
								DataFeedAggArguments args) {
		if(transform_str.empty()) {
			if(input_type == target_output_type)
				return opaque_feed;
			return nullptr;
		}
		auto avail_transforms = get_factories();
		SharedPtr <FeedTransformFactoryBase> transform_factory = avail_transforms[input_type][target_output_type][transform_str];
		if (!transform_factory) {
			Log::Message(Log::LT_ERROR, "Data transformation function not found: %s", transform_str.c_str());
			return nullptr;
		}
		auto raw_ptr = transform_factory->get_raw(opaque_feed, args);
		return raw_ptr;
	}

	template<>
	UniquePtr <DataFeedBase<Vector2f>>
	TransformFeedString(const String &data_str) {
		return MakeUnique<DataFeedString<Vector2f>>(data_str);
	}

}