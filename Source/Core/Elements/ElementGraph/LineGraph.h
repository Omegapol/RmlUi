//
// Created by Mateusz Raczynski on 4/17/2021.
//

#ifndef RMLUI_LINEGRAPH_H
#define RMLUI_LINEGRAPH_H

#include "BaseGraph.h"
#include "DataFeed.h"

namespace Rml {
	class LineGraph : public BaseGraphImpl<Vector2f> {
	public:
		RMLUI_RTTI_DefineWithParent(CandleGraph, BaseGraphImpl<Vector2f>)
		void GenerateGeometryInit() override;
		void GenerateGeometryStart(const Vector2<float> &from) override;

		explicit LineGraph(const String& tag): BaseGraphImpl<Vector2f>(tag)
		{};

		bool IsPointWithinElement(Vector2f point) override;

		GraphRenderMetadata
		GenerateGeometryPart(Vertex *vertices, int *indices, const Vector2<float> &next,
							 int index_offset, Colourb colour, Vector2f offset, Rml::Vector2f scale, double width) override;

		GraphRenderMetadata GetMetadata(unsigned int input_size) override;

	protected:
		bool current_set = false;
		Vector2f current;
	};
}

#endif //RMLUI_LINEGRAPH_H
