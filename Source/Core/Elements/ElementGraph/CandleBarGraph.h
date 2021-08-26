//
// Created by Mateusz Raczynski on 5/16/2021.
//

#ifndef RMLUI_CANDLEBARGRAPH_H
#define RMLUI_CANDLEBARGRAPH_H



#include "DataFeed.h"
#include "CandleGraph.h"

namespace Rml {
	class CandleBarGraph : public CandleGraph {
	public:
		explicit CandleBarGraph(const String& tag): CandleGraph(tag)
		{};

		Rml::GraphRenderMetadata
		GenerateGeometryPart(Vertex *vertices, int *indices, const CandleStickData &next,
							 int index_offset, Colourb colour, Vector2f offset, Rml::Vector2f scale, double width) override;

		GraphRenderMetadata GetMetadata(unsigned int input_size) override;
	};
}

#endif //RMLUI_CANDLEBARGRAPH_H
