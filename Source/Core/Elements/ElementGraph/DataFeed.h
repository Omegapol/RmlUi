//
// Created by Mateusz Raczynski on 4/15/2021.
//

#ifndef RMLUI_DATAFEED_H
#define RMLUI_DATAFEED_H

#include "RmlUi/Core/DataVariable.h"
#include "../common.h"
#include "DataGraphTypes.h"
#include "RmlUi/Core/Elements/DataFeedBase.h"
#include <cstring>
#include <limits>

#include "DataFeedTypeCheck.h"

#define CHUNK_SIZE 40

namespace Rml {
	template<typename DataType>
	class DataFeed: public DataFeedBase<DataType> {
	protected:
		DataType GetFrom(Rml::DataVariable var) override;
		using Chunk = typename DataFeedBase<DataType>::Chunk;
		using ChunkList = typename DataFeedBase<DataType>::ChunkList;
	public:

		const ChunkList GetData(Vector2f xvis) override;

		const ChunkList GetElement(unsigned int idx) override;

		Rml::Vector<DataType> GetNeighboursXFromCache(float x) override;

		const ChunkList GetData(double xpoint) override;

	public:
		~DataFeed() override = default;

		DataFeed() = default;

		DataFeed(Rml::DataVariable source, UniquePtr<TypeChecker<DataType>> type_parser): DataFeedBase<DataType>(source) {
			this->data_parser = std::move(type_parser);
		};

		virtual const ChunkList GetData();

		virtual const ChunkList GetData(unsigned int start, unsigned int end);

		inline double GetX(DataType el);

	private:
		virtual const ChunkList _GetData(Vector2f xvis, bool cache);

		virtual const ChunkList _GetData(unsigned int start, unsigned int end, bool cache);

	protected:
		ChunkList cached;
		Rml::UnorderedMap<double, Rml::SharedPtr<Chunk>> cached_dict;

		UniquePtr<TypeChecker<DataType>> data_parser;

	};

	template<typename DataType>
	inline double DataFeed<DataType>::GetX(DataType el) {
		return el.x;
	}

	template<>
	inline double DataFeed<CandleStickData>::GetX(CandleStickData el) {
		return el.time;
	}

	template<typename DataType>
	const typename DataFeed<DataType>::ChunkList DataFeed<DataType>::GetData() {
		return GetData(0, this->Size());
	}

	template<typename DataType>
	const typename DataFeed<DataType>::ChunkList DataFeed<DataType>::GetData(double xpoint) {
		return _GetData({(float) xpoint, (float) xpoint}, false);
	}

	template<typename DataType>
	const typename DataFeed<DataType>::ChunkList DataFeed<DataType>::GetElement(unsigned int idx) {
		return _GetData(idx, idx + 1, false);
	}

	template<typename DataType>
	const typename DataFeed<DataType>::ChunkList DataFeed<DataType>::GetData(unsigned int start, unsigned int end) {
		return _GetData(start, end, true);
	}

	template<typename DataType>
	const typename DataFeed<DataType>::ChunkList
	DataFeed<DataType>::_GetData(unsigned int start, unsigned int end, bool cache) {
		auto cl = DataFeed::ChunkList();
		auto idx = start;

		while (idx < end) {
			auto base = idx / CHUNK_SIZE * CHUNK_SIZE;
			auto next = std::min(base + CHUNK_SIZE, end);
			if (cache)
				next = std::min(base + CHUNK_SIZE, (unsigned int) this->Size());
			else
				base = std::max(base, start);

			auto chunk_it = cached_dict.find(base);
			bool use_new = false;
			use_new = chunk_it == cached_dict.end() ||
					  chunk_it->second->start != base ||
					  chunk_it->second->end != next;
			if (use_new) {
				auto c = Rml::MakeShared<DataFeed<DataType>::Chunk>();
				auto &vec = c->data;
				auto end_chunk_idx = std::min(next, (unsigned int) this->Size());
				for (auto i = idx; i < end_chunk_idx; ++i) {
					auto value = this->Get(i);
					vec.push_back(value);
				}
				double lower_bound = GetX(this->Get(idx));
				c->lower_bound = lower_bound;
				c->higher_bound = GetX(this->Get(end_chunk_idx - 1));

				c->start = idx;
				c->end = next;
				c->base = base;
				cl.push_back(c);
				if (cache) {
					cached_dict[base] = c;
				}
			} else {
				cl.push_back(chunk_it->second);
			}

			idx = next;
		}

		if(!cl.empty()) {
			Rml::SharedPtr<Chunk> last_cached_chunk = cl.back();
			auto last_el =  this->Get(last_cached_chunk->end-1);
			if(memcmp(&last_cached_chunk->data.back(), &last_el, sizeof(last_el)) != 0)
			{
				last_cached_chunk->data.back() = last_el;
			}
		}

		if (cache) {
			this->cached = cl;
		}
		return cl;
	}

	template<typename DataType>
	const typename DataFeed<DataType>::ChunkList DataFeed<DataType>::GetData(Vector2f xvis) {
		return _GetData(xvis, true);
	}

	template<typename DataType>
	const typename DataFeed<DataType>::ChunkList DataFeed<DataType>::_GetData(Vector2f xvis, bool cache) {
		int size = this->Size();

		auto TRK = 0;
		auto start_lower = 0u;
		auto start_upper = 0u;
		auto end_lower = size;
		auto end_upper = size;
		if (end_lower != 0) {
			while (end_lower - start_lower > 1) {
				auto mid = (end_lower + start_lower) / 2;
				auto pt = this->Get(mid);
				auto X = GetX(pt);
				if (X > xvis.x)
					end_lower = mid;
				if (X <= xvis.x)
					start_lower = mid;
				++TRK;
				if (TRK > 10000) {
					break;
				}
			}
			while (end_upper - start_upper > 1) {
				auto mid = (end_upper + start_upper) / 2;
				auto pt = this->Get(mid);
				auto X = GetX(pt);
				if (X > xvis.y)
					end_upper = mid;
				if (X <= xvis.y)
					start_upper = mid;
				++TRK;
				if (TRK > 10000) {
					break;
				}
			}

			if (end_upper == size)
				end_upper -= 1;
		}
		return _GetData(start_lower, std::min(end_upper + 2, size), cache);
//		return _GetData(start_lower, std::min(end_upper+2, size), cache);
	}

	template<typename DataType>
	inline DataType DataFeed<DataType>::GetFrom(Rml::DataVariable var) {
		return data_parser->get_sub(var);
	}

	template<>
	inline Vector2f DataFeed<Vector2f>::GetFrom(Rml::DataVariable var) {
		Variant tmp;
		float x = 0, y = 0;
		var.Child(String("x")).Get(tmp);
		tmp.GetInto(x);
		var.Child(String("y")).Get(tmp);
		tmp.GetInto(y);
		return {x, y};
	}

	template<typename DataType>
	Vector<DataType> DataFeed<DataType>::GetNeighboursXFromCache(float x) {

		if(!this->cached.empty() && !this->cached[0]->data.empty()) {
			auto prev = this->cached[0]->data[0];

			//check each chunk if it contains values close to x
			for (const Rml::SharedPtr<Chunk> &cached_chunk: this->cached) {
				auto first = cached_chunk->data[0];

				//check if searched x is between chunks
				if(GetX(prev) < x && x < GetX(first))
					return {prev, first};

				//check if searched x is inside the chunk
				if (cached_chunk->lower_bound < x && x < cached_chunk->higher_bound) {
					for (auto &cur: cached_chunk->data) {
						if (GetX(prev) < x && x < GetX(cur)) {
							return {prev, cur};
						}
						prev = cur;
					}
				}
				prev = cached_chunk->data[cached_chunk->data.size() - 1];
			}
		}
		//todo: can we do it?
		return {};

//		auto data = this->GetData(x);
//		DataType first, second;
//		int found_state = 0;
////		for(auto v: data[0]->data)
////		{
////			if(GetX(v) > x || (found_state && GetX(v) > x - 1.5)) {
////				second = v;
////				found_state += 1;
////				break;
////			}
////			first = v;
////			found_state = 1;
////		}
//
//		auto TRK = 0;
//		auto start = 0u;
//		if (data.size() == 0)
//			return {};
//		auto &data1 = data[0]->data;
//		auto end = data1.size();
//
//
//		if (end != 0) {
//			while (end - start > 1) {
//				auto mid = (end + start) / 2;
//				auto pt = data1[mid];
//				auto X = GetX(pt);
//				if (X > x)
//					end = mid;
//				if (X <= x)
//					start = mid;
//				++TRK;
//				if (TRK > 10000) {
//					break;
//				}
//			}
//
//			if (end == data1.size())
//				return {};
//
//			first = data1[start];
//			second = data1[end];
//			found_state = 2;
//		}
//
//		if (found_state != 2)
//			return {};
//		else
//			return {first, second};
	}

	template<>
	inline Vector4f DataFeed<Vector4f>::GetFrom(Rml::DataVariable var) {
		Variant tmp;
		float h = 0, l = 0, en = 0, ex = 0;
		var.Child(String("h")).Get(tmp);
		tmp.GetInto(h);
		var.Child(String("l")).Get(tmp);
		tmp.GetInto(l);
		var.Child(String("en")).Get(tmp);
		tmp.GetInto(en);
		var.Child(String("ex")).Get(tmp);
		tmp.GetInto(ex);
		return {h, l, en, ex};
	}

	template<>
	inline CandleStickData DataFeed<CandleStickData>::GetFrom(Rml::DataVariable var) {
		Variant tmp;
		float h = 0, l = 0, en = 0, ex = 0;
		double time = 0;
		var.Child(String("h")).Get(tmp);
		tmp.GetInto(h);
		var.Child(String("l")).Get(tmp);
		tmp.GetInto(l);
		var.Child(String("en")).Get(tmp);
		tmp.GetInto(en);
		var.Child(String("ex")).Get(tmp);
		tmp.GetInto(ex);
		var.Child(String("time")).Get(tmp);
		tmp.GetInto(time);
		return {h, l, en, ex, time};
	}


	template
	class DataFeed<Vector2f>;

	template
	class DataFeed<Vector4f>;

	template
	class DataFeed<CandleStickData>;


	template<typename DataType>
	class DataFeedString : public DataFeed<DataType> {
		String _str;
		Vector<DataType> cached;

	public:
		DataFeedString(String str_data);

		Vector<DataType> &Extract() {
			auto vec = Vector<DataType>();
			cached = vec;
			return cached;
		}

	protected:
		int Size() override;

		DataType Get(int i) override;
	};

	template<>
	Vector<Vector2f> &DataFeedString<Vector2f>::Extract();

	template<typename DataType>
	DataFeedString<DataType>::DataFeedString(String source) {
		_str = source;
		Extract();
	}

	template<typename DataType>
	int DataFeedString<DataType>::Size() {
		// DataAddressEntry can be either int or string, so we have to use int for indexing elements
		auto size = cached.size();
		RMLUI_ASSERT(size < static_cast<size_t>(std::numeric_limits<int>::max()));
		return static_cast<int>(size);
	}

	template<typename DataType>
	DataType DataFeedString<DataType>::Get(int i) {
		return cached[i];
	}

	template
	class DataFeedString<Vector2f>;

}

#endif //RMLUI_DATAFEED_H
