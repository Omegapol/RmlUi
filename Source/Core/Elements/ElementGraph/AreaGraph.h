//
// Created by Mateusz Raczynski on 5/12/2021.
//

#ifndef RMLUI_AREAGRAPH_H
#define RMLUI_AREAGRAPH_H

#include "LineGraph.h"

namespace Rml {
	class AreaGraph : public LineGraph {
	public:
		explicit AreaGraph(const String& tag): LineGraph(tag)
		{};

		bool IsPointWithinElement(Vector2f point) override;

		Rml::GraphRenderMetadata
		GenerateGeometryPart(Vertex *vertices, int *indices, const Vector2<float> &next,
							 int index_offset, Colourb colour, Vector2f offset, Rml::Vector2f scale,
							 double width) override;

		GraphRenderMetadata GetMetadata(unsigned int input_size) override;
	};
}

#endif //RMLUI_AREAGRAPH_H
