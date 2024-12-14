//
// Created by Mateusz Raczynski on 5/16/2021.
//

#include "CandleBarGraph.h"
#include "RmlUi/Core/GeometryUtilities.h"
#include "RmlUi/Core/MeshUtilities.h"

Rml::GraphRenderMetadata Rml::CandleBarGraph::GenerateGeometryPart(Rml::Vertex *vertices, int *indices,
											   const Rml::CandleStickData &current, int index_offset, Rml::Colourb colour,
											   Rml::Vector2f offset, Rml::Vector2f scale, double width) {
	if(current.entry < current.exit)
	{
		colour.red = 0;
		colour.green = 215;
		colour.blue = 0;
	}
	else {
		colour.red = 215;
		colour.green = 0;
		colour.blue = 0;
	}
	ColourbPremultiplied colour_premultiplied = colour.ToPremultiplied();

	auto wick_width = 3.f;
	MeshUtilities::GenerateLineGraph(vertices, indices, {(float) current.time, current.low},
										 {(float) current.time, current.high},
										 colour_premultiplied, wick_width, Vector2f{}, Vector2f{}, scale, index_offset, offset);
	MeshUtilities::GenerateLineGraph(vertices+4, indices+6, {(float) current.time - (float) width / 2 * scale.x, current.entry},
										 {(float) current.time, current.entry},
										 colour_premultiplied, wick_width, Vector2f{}, Vector2f{}, scale, index_offset+4, offset);
	MeshUtilities::GenerateLineGraph(vertices+8, indices+12, {(float) current.time, current.exit},
										 {(float) current.time + (float) width / 2 * scale.x, current.exit},
										 colour_premultiplied, wick_width, Vector2f{}, Vector2f{}, scale, index_offset+8, offset);
	Rml::GraphRenderMetadata res;
	res.vertices_delta=12;
	res.indices_delta=18;
	return res;
}

Rml::GraphRenderMetadata Rml::CandleBarGraph::GetMetadata(unsigned int input_size) {
	auto meta = Rml::GraphRenderMetadata();
	meta.vertices_delta = 12;
	meta.indices_delta = 18;
	meta.target_size = input_size;
	return meta;
}
