//
// Created by Mateusz Raczynski on 7/17/2021.
//

#ifndef RMLUI_DATAFEEDBASE_H
#define RMLUI_DATAFEEDBASE_H

#include <RmlUi/Core/DataVariable.h>

namespace Rml {
	template<typename DataType>
	class DataFeedBase {
	public:
		virtual ~DataFeedBase() = default;

		DataFeedBase() = default;

		DataFeedBase(Rml::DataVariable _source) {
			source = _source;
		}

		Rml::DataVariable source;
		using Primitive = DataType;
		struct Chunk {
			Rml::Vector<DataType> data;
			bool updated = true;
			unsigned int start;
			unsigned int end;
			double base;

			double lower_bound;
			double higher_bound;
		};
		using ChunkList = Rml::Vector<Rml::SharedPtr<Chunk>>;

		virtual const ChunkList GetData(Vector2f xvis) = 0;

		virtual const ChunkList GetData(double x) = 0;

		virtual const ChunkList GetElement(unsigned int start) = 0;

		virtual double GetInterval() {
			return 0.0;
		}

		virtual Rml::Vector<DataType> GetNeighboursXFromCache(float x) = 0;

		virtual DataType Get(int i) {
			auto var = source.Child(i);
			auto value = GetFrom(var);
			return value;
		}

		virtual int Size() {
			return source.Size();
		}

	protected:
		virtual DataType GetFrom(Rml::DataVariable RMLUI_UNUSED_PARAMETER(var)) {
			RMLUI_UNUSED(var);
			return DataType();
		};
	};
}
#endif //RMLUI_DATAFEEDBASE_H
