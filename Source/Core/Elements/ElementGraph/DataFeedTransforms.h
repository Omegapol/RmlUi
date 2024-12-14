//
// Created by Mateusz Raczynski on 7/14/2021.
//

#ifndef RMLUI_DATAFEEDTRANSFORMS_H
#define RMLUI_DATAFEEDTRANSFORMS_H

#include "RmlUi/Core/Elements/DataFeedBase.h"
#include "DataGraphTypes.h"
#include "MakeFeed.h"

namespace Rml {
	namespace impl {

		void *TransformFeedProtImpl(void *opaque_feed, FamilyId input_type, FamilyId target_output_type,
									const String &transform_str,
									DataFeedAggArguments args);

		class FeedTransformFactoryBase {
			friend void *TransformFeedProtImpl(void *opaque_feed, FamilyId input_type, FamilyId target_output_type,
											   const String &transform_str,
											   DataFeedAggArguments args);

			virtual void *get_raw(void * /*raw_feed*/, DataFeedAggArguments /*args*/) {
				return nullptr;
			};

		public:
			virtual ~FeedTransformFactoryBase() = default;

			virtual String GetName() = 0;

			virtual void GetId(FamilyId &input, FamilyId &output) {
				input = (FamilyId) - 1;
				output = (FamilyId) - 1;
			}
		};

		typedef UnorderedMap <FamilyId, UnorderedMap<FamilyId,
				UnorderedMap < String, SharedPtr < FeedTransformFactoryBase>>>>
		FactoryStorageType;

	}

	template<typename Input, typename Output>
	class FeedTransformFactory : public impl::FeedTransformFactoryBase {
	private:

		void *get_raw(void *raw_feed, DataFeedAggArguments args) override {
			auto feed = (DataFeedBase<Input> *) raw_feed;
			auto ptr = get(feed, args);
			return ptr.release();
		}

	public:
		void GetId(FamilyId &input, FamilyId &output) override {
			input = Family<Input>::Id();
			output = Family<Output>::Id();
		}

		virtual UniquePtr <DataFeedBase<Output>> get(DataFeedBase<Input> *raw_feed, DataFeedAggArguments args) = 0;
	};

	template<typename T, typename I, typename O>
	class GenericAggFactory : public FeedTransformFactory<I, O> {
		String name;
	public:
		explicit GenericAggFactory(const String &_name) {
			this->name = _name;
		}

		String GetName() override {
			return this->name;
		}

		UniquePtr <DataFeedBase<O>> get(DataFeedBase<I> *raw_feed, DataFeedAggArguments args) override {
			if(args.agg_size <= 0) {
				Log::Message(Log::LT_ERROR, "Invalid transf-step: 0");
				return UniquePtr <DataFeedBase<O>>();
			}
			auto feed = UniquePtr<DataFeedBase<I>>(raw_feed);
			return std::move(MakeUnique<T>((int) args.agg_size, std::move(feed)));
		}
	};

	void RegisterFactory(impl::FactoryStorageType & storage, SharedPtr < impl::FeedTransformFactoryBase > factory);

	template<typename TargetType, typename InputType>
	UniquePtr <DataFeedBase<TargetType>>
	TransformFeed(UniquePtr <DataFeedBase<InputType>> input_feed, const String &transform_str,
				  DataFeedAggArguments args) {
		void *res = impl::TransformFeedProtImpl(input_feed.release(), Family<InputType>::Id(),
												Family<TargetType>::Id(),
												transform_str, args);
		if (res) {
			auto *ptr = reinterpret_cast<DataFeedBase<TargetType> *>(res);
			auto ptr_res = UniquePtr<DataFeedBase<TargetType>>(ptr);
			return ptr_res;
		}
		return UniquePtr<DataFeedBase<TargetType>>();
	}

	template<typename T>
	UniquePtr <DataFeedBase<T>>
	TransformFeedString(const String &data_str);

	template<>
	UniquePtr <DataFeedBase<Vector2f>>
	TransformFeedString(const String &data_str);
}

#endif //RMLUI_DATAFEEDTRANSFORMS_H
