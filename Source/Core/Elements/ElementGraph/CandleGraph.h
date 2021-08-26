//
// Created by Mateusz Raczynski on 5/15/2021.
//

#ifndef RMLUI_CANDLEGRAPH_H
#define RMLUI_CANDLEGRAPH_H

#include "BaseGraph.h"
#include "DataFeed.h"

namespace Rml {
	class CandleGraph : public BaseGraphImpl<CandleStickData> {
	public:
		explicit CandleGraph(const String& tag): BaseGraphImpl<CandleStickData>(tag)
		{};

		bool IsPointWithinElement(Vector2f point) override;

		GraphRenderMetadata
		GenerateGeometryPart(Vertex *vertices, int *indices, const CandleStickData &next,
							 int index_offset, Colourb colour, Vector2f offset, Rml::Vector2f scale, double width) override;

		GraphRenderMetadata GetMetadata(unsigned int input_size) override;

	protected:
		float GetWidth() override;
	};
}


#endif //RMLUI_CANDLEGRAPH_H
