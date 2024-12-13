//
// Created by Mateusz Raczynski on 7/14/2021.
//

#ifndef RMLUI_DATAFEEDAGGREGATESTEP_H
#define RMLUI_DATAFEEDAGGREGATESTEP_H

#include "DataFeed.h"

namespace Rml {
	template<typename DataType, typename OutputType>
	struct DefaultAgg {
	public:
		static void Finish(OutputType &res, DataType first, DataType end, int count) {};

		static void Init(OutputType &res, DataType first) {};
	};

	template<typename InputType, typename OutputType, typename AggClass>
	class DataFeedAggregateStep : public DataFeed<OutputType> {
	protected:
		double agg_step;

	public:
		DataFeedAggregateStep(double agg_step, UniquePtr <DataFeedBase<InputType>> inner) : agg_step(agg_step) {
			this->inner = std::move(inner);
		}

		double GetInterval() override {
			return agg_step;
		}

		int Size() override {
			int size = inner->Size();
			if (size == 0)
				return 0;
			auto first = inner->Get(0);
			auto last = inner->Get(size - 1);
			auto start_x = GetX(first) - std::fmod(GetX(first), agg_step);
			auto end_x = GetX(last) + (agg_step - std::fmod(GetX(last), agg_step));
			auto delta = end_x - start_x;
			return static_cast<int>(delta / agg_step);
		}

		OutputType Get(int i) override {
			auto first = inner->Get(0);
			double first_x = GetX(first);
			auto start = first_x - std::fmod(first_x, agg_step) + agg_step * i;
			auto end = start + agg_step;
			auto chunks = inner->GetData(start);
			auto start_idx = chunks[0]->start;

			int end_idx = (int) start_idx;
			return AggClass::AggFunc(inner.get(), end_idx, end);
		}

	protected:
		UniquePtr <DataFeedBase<InputType>> inner;
	};


	template<typename DataType, typename OutputType, typename InnerAggClass,
			typename InitClass = DefaultAgg<OutputType, DataType>, typename FinishClass = DefaultAgg<OutputType, DataType>>
	struct GenericAggStepImpl {
		static OutputType AggFunc(DataFeedBase <DataType> *feed, int &start_idx, double end) {
			OutputType res;
			DataType prev = feed->Get(std::max(start_idx-1, 0));
			DataType first = feed->Get(start_idx);
			InitClass::Init(res, first);

			SetX(res, end);
			auto idx = start_idx;
			DataType last, next;
			next = feed->Get(idx);
			auto x = GetX(first);
			int size = feed->Size();
			while (x < end) {
				last = next;
				InnerAggClass::AggFuncInner(res, last);

				++idx;
				if(idx >= size)
					break;
				next = feed->Get(idx);
				x = GetX(next);
			}

			FinishClass::Finish(res, prev, first, last, idx - 1 - start_idx);

			start_idx = idx;
			return res;
		}
	};

	template<typename...>
	struct GenericAggStep;

	template<typename DataType, typename OutputType, typename T>
	struct GenericAggStep<DataType, OutputType, T> {
		using Def = DataFeedAggregateStep<DataType, OutputType,
				GenericAggStepImpl<DataType, OutputType, T, T, T>>;
	};

	template<typename DataType, typename OutputType, typename A, typename B, typename C>
	struct GenericAggStep<DataType, OutputType, A, B, C> {
		using Def = DataFeedAggregateStep<DataType, OutputType,
				GenericAggStepImpl<DataType, OutputType, A, B, C>>;
	};

	struct AggVector2Candles {
	public:
		static void Init(CandleStickData &res,  const Vector2f& first) {
			res.low = first.y;
			res.high = first.y;
		}

		static void AggFuncInner(CandleStickData &res, const Vector2f &cur) {
			res.low = std::min(res.low, cur.y);
			res.high = std::max(res.high, cur.y);
		}

		static void Finish(CandleStickData &res, const Vector2f& /*prev*/,  const Vector2f& first,  const Vector2f& end, int /*count*/) {
			res.entry = first.y;
			res.exit = end.y;
		}
	};

	typedef GenericAggStep<Vector2f, Vector2f, AggSum,
			DefaultAgg<Vector2f, Vector2f>, DefaultAgg<Vector2f, Vector2f>>::Def
			DataFeedAggStepSum;

	typedef GenericAggStep<Vector2f, CandleStickData, AggVector2Candles>::Def
			DataFeedAggStepVector2Candles;


}

#endif //RMLUI_DATAFEEDAGGREGATESTEP_H
