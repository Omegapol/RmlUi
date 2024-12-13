//
// Created by Mateusz Raczynski on 5/15/2021.
//

#include "CandleGraph.h"
#include "RmlUi/Core/GeometryUtilities.h"

Rml::GraphRenderMetadata Rml::CandleGraph::GenerateGeometryPart(Rml::Vertex *vertices, int *indices,
											const Rml::CandleStickData &current, int index_offset, Rml::Colourb colour,
											Rml::Vector2f offset, Rml::Vector2f scale, double width) {
	auto wick_colour = colour;
	wick_colour.red = 255;
	wick_colour.green = 255;
	wick_colour.blue = 255;
	auto wick_width = std::min((float) width / 12.f, 3.f);
	wick_width = std::max(wick_width, 1.5f);
	MeshUtilities::GenerateLineGraph(vertices, indices, {(float) current.time, current.low},
										 {(float) current.time, current.high},
										 wick_colour.ToPremultiplied(), wick_width, Vector2f{}, Vector2f{}, scale, index_offset, offset);

	if (current.entry < current.exit) {
		colour.red = 0;
		colour.green = 215;
		colour.blue = 0;
	} else {
		colour.red = 215;
		colour.green = 0;
		colour.blue = 0;
	}
	auto body_width = width;
	body_width = std::max((float) body_width, 2.f);
	MeshUtilities::GenerateLineGraph(vertices + 4, indices + 6, {static_cast<float>(current.time), current.entry},
										 {static_cast<float>(current.time), current.exit},
										 colour.ToPremultiplied(), static_cast<float>(body_width), Vector2f{}, Vector2f{}, scale, index_offset + 4, offset);

	Rml::GraphRenderMetadata res;
	res.vertices_delta=8;
	res.indices_delta=12;
	return res;
}

bool Rml::CandleGraph::IsPointWithinElement(Rml::Vector2f point) {
	auto parent = GetParentNode();

	auto transl = parent->GetAbsoluteOffset(BoxArea::Content).Round();
	Vector2f quad_size = parent->GetBox().GetSize(BoxArea::Content).Round();
	if (point.x > transl.x && point.x < transl.x + quad_size.x &&
		point.y > transl.y && point.y < transl.y + quad_size.y) {
		auto view_point = ScreenPointToView(point, transl);

		if(!this->feed)
			return false;

		auto vec = this->feed->GetNeighboursXFromCache(view_point.x);
		if (vec.empty())
			return false;
		auto first = vec[0];
		auto second = vec[1];

		return view_point.y < first.high && view_point.y > first.low ||
			   view_point.y < second.high && view_point.y > second.low;
	}
	return false;
}

Rml::GraphRenderMetadata Rml::CandleGraph::GetMetadata(unsigned int input_size) {
	auto meta = Rml::GraphRenderMetadata();
	meta.vertices_delta = 8;
	meta.indices_delta = 12;
	meta.target_size = input_size;
	return meta;
}

float Rml::CandleGraph::GetWidth() {
	auto interval = GetInterval();
	auto padding_attr = this->GetAttribute("padding");
	auto padding_f = 0.35f;
	if (padding_attr) {
		auto padding_str = padding_attr->Get<String>();
		auto padding = std::atoi(padding_str.substr(0, padding_str.size() - 1).c_str());
		padding_f = (float) padding / 100.f;
	}
	auto ratios = GetRatios(GetCanvasSize());
	return static_cast<float>(interval / ratios.x * (1.f - padding_f));
}
