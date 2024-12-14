//
// Created by Mateusz Raczynski on 6/18/2021.
//

#ifndef RMLUI_DATAFEEDAGGREGATE_H
#define RMLUI_DATAFEEDAGGREGATE_H

#include "DataFeed.h"
#include "DataGraphTypes.h"
#include <cmath>

namespace Rml {

	template<typename InputType, typename OutputType, typename AggClass>
	class DataFeedAggregate : public DataFeed<OutputType> {
	protected:
		int agg_step;

	public:
		DataFeedAggregate(int aggSize, UniquePtr <DataFeedBase<InputType>> inner) : agg_step(aggSize) {
			this->inner = std::move(inner);
		}

		double GetInterval() override {
			if (inner->Size() >= 2) {
				auto first = inner->Get(0);
				auto second = inner->Get(1);
				return (GetX(second) - GetX(first)) * agg_step / 2.f;
			}
			return 0.0;
		}

		int Size() override {
			return inner->Size() / agg_step + (inner->Size() % agg_step > 0 ? 1 : 0);
		}

		OutputType Get(int i) override {
			if (i == (Size() - 1)) {
				auto res = AggClass::AggFunc(inner.get(), i * agg_step, inner->Size() - i * agg_step);

				auto inner_size = inner->Size();
				if (inner_size >= 2) {
					double last;
					double avg_distance = 0.f;
					auto end = i * agg_step;
					if (i > 0) {
						last = GetX(inner->Get(std::max(i * agg_step - 1, 0)));
					} else {
						auto first = inner->Get(0);
						last = GetX(first);
						end = inner_size;
					}

					// assume that interval between values is always the same, trying to estimate it
					auto k = 1;
					while (k < agg_step - 2) {
						if (k + 3 > inner_size)
							break;
						++k;
						auto first = inner->Get(end - (k + 1));
						auto second = inner->Get(end - (k));
						avg_distance += GetX(second) - GetX(first);
					}
					avg_distance /= k;

					SetX(res, last + (avg_distance) * agg_step);
				}
				return res;
			}
			return AggClass::AggFunc(inner.get(), i * agg_step, agg_step);
		}

	protected:
		UniquePtr <DataFeedBase<InputType>> inner;
	};

	template<typename DataType, typename OutputType, typename InnerAggClass>
	struct GenericAgg {
		static OutputType AggFunc(DataFeedBase<DataType> *feed, int start, int size) {
			OutputType res;
			res.x = feed->Get(start).x;
			for (auto j = start; j < start + size; ++j) {
				InnerAggClass::AggFuncInner(res, feed->Get(j));
			}
			return res;
		}
	};

	RMLUICORE_API void Div(Vector2f &obj, float div);

	template<typename T>
	void SetX(T &obj, double x) {
		obj.x = static_cast<decltype(obj.x)>(x);
	}

	template<>
	RMLUICORE_API void SetX(CandleStickData &obj, double x);

	template<typename T>
	inline double GetX(T el) {
		return el.x;
	}

	template<>
	inline double GetX(CandleStickData el);

	struct AggSum {
	public:
		static void AggFuncInner(Vector2f &res, const Vector2f &cur) {
			res.y = res.y + cur.y;
		}
	};

	typedef DataFeedAggregate<Vector2f, Vector2f,
			GenericAgg<Vector2f, Vector2f, AggSum>> DataFeedAggSum;

	struct AggMax {
	public:
		static void AggFuncInner(Vector2f &res, const Vector2f &cur) {
			res.y = std::max(res.y, cur.y);
		}
	};

	typedef DataFeedAggregate<Vector2f, Vector2f,
			GenericAgg<Vector2f, Vector2f, AggMax>> DataFeedAggMax;

	struct AggMin {
	public:
		static void AggFuncInner(Vector2f &res, const Vector2f &cur) {
			res.y = std::min(res.y, cur.y);
		}
	};

	typedef DataFeedAggregate<Vector2f, Vector2f,
			GenericAgg<Vector2f, Vector2f, AggMin>> DataFeedAggMin;

	template<typename DataType, typename OutputType>
	struct AggAvgFunc {
	public:
		static OutputType AggFunc(DataFeedBase<DataType> *feed, int start, int size) {
			OutputType res;
			auto end_val = feed->Get(start + size - 1);
			for (auto j = start; j < start + size; ++j) {
				AggSum::AggFuncInner(res, feed->Get(j));
			}
			Div(res, (float) size);
			res.x = end_val.x;
			return res;
		}
	};

	typedef DataFeedAggregate<Vector2f, Vector2f,
			AggAvgFunc<Vector2f, Vector2f>> DataFeedAggVecAvg;
	//todo: typedef aggregates for avg

	struct AggVectorFunc {
	public:
		static CandleStickData AggFunc(DataFeedBase<Vector2f> *feed, int start, int size) {
			CandleStickData res;
			auto prev_idx = 0;
			if (start > 0)
				prev_idx = start - 1;
			const auto &prev = feed->Get(prev_idx);
			const auto &started = feed->Get(start);
			const auto &ended = feed->Get(start + size - 1);
			res.time = ended.x;
			res.entry = prev.y;
			res.low = started.y;
			res.high = started.y;
			for (auto j = start + 1; j < start + size; ++j) {
				const auto &el = feed->Get(j);
				res.low = std::min(res.low, el.y);
				res.high = std::max(res.high, el.y);
			}
			res.exit = ended.y;
			return res;
		}
	};

	typedef DataFeedAggregate<Vector2f, CandleStickData, AggVectorFunc> DataFeedAggVector2Candles;

	struct AggCandleFunc {
	public:
		static CandleStickData AggFunc(DataFeedBase<CandleStickData> *feed, int start, int size) {
			CandleStickData res;
			const CandleStickData &started = feed->Get(start);
			const CandleStickData &ended = feed->Get(start + size - 1);
			res = started;
			res.time = ended.time;
			for (auto j = start + 1; j < start + size; ++j) {
				const auto &el = feed->Get(j);
				res.low = std::min(res.low, el.low);
				res.high = std::max(res.high, el.high);
			}
			res.exit = ended.exit;
			return res;
		}
	};

	typedef DataFeedAggregate<CandleStickData, CandleStickData, AggCandleFunc> DataFeedAggCandles2Candles;
}


#endif //RMLUI_DATAFEEDAGGREGATE_H
