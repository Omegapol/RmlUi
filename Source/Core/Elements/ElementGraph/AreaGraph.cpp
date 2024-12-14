//
// Created by Mateusz Raczynski on 5/12/2021.
//

#include "AreaGraph.h"
#include "RmlUi/Core/GeometryUtilities.h"
#include "RmlUi/Core/MeshUtilities.h"
#include <cmath>

float FindYCrossing(const Rml::Vector2f &a, const Rml::Vector2f &b) {
	auto delta_x = b.x - a.x;
	auto delta_y = b.y - a.y;
	auto eff = delta_y / delta_x;
	auto req_delta_x = a.y / eff;
	return a.x + std::abs(req_delta_x);
}

bool Rml::AreaGraph::IsPointWithinElement(Rml::Vector2f point) {
	auto parent = GetParentNode();

	auto transl = parent->GetAbsoluteOffset(BoxArea::Content).Round();
	Vector2f quad_size = parent->GetBox().GetSize(BoxArea::Content).Round();
	if (point.x > transl.x && point.x < transl.x + quad_size.x &&
		point.y > transl.y && point.y < transl.y + quad_size.y) {
		auto view_point = ScreenPointToView(point, transl);

		auto vec = this->feed->GetNeighboursXFromCache(view_point.x);
		if (vec.empty())
			return false;
		auto first = vec[0];
		auto second = vec[1];

		return isUnderSegmentTowardsX(first, second, view_point);
	}
	return false;
}

Rml::GraphRenderMetadata Rml::AreaGraph::GenerateGeometryPart(Rml::Vertex *vertices, int *indices,
															  const Rml::Vector2<float> &next, int index_offset,
															  Rml::Colourb colour,
															  Rml::Vector2f offset, Rml::Vector2f scale, double width) {
	Rml::GraphRenderMetadata res;
	if(current_set) {
		Vector2f pts[] = {{current.x / scale.x + offset.x, offset.y},
						  {current.x / scale.x + offset.x, current.y / scale.y + offset.y},
						  {next.x / scale.x + offset.x,    offset.y}};
		Vector2f pts2[] = {{next.x / scale.x + offset.x,    offset.y},
						   {current.x / scale.x + offset.x, current.y / scale.y + offset.y},
						   {next.x / scale.x + offset.x,    next.y / scale.y + offset.y}};
		if (std::signbit(current.y) != std::signbit(next.y)) {
			auto mid_x = FindYCrossing(current, next);
			pts[2] = {mid_x / scale.x + offset.x, offset.y};
			pts2[1] = {mid_x / scale.x + offset.x, offset.y};
		}
		Vector2f tex[] = {{0.0f, 0.0f},
						  {0.0f, 0.0f},
						  {0.0f, 0.0f}};
		Vector2f tex2[] = {{0.0f, 0.0f},
						   {0.0f, 0.0f},
						   {0.0f, 0.0f}};
		MeshUtilities::GenerateTriangle(vertices, indices,
		 									pts, colour.ToPremultiplied(),
		 									tex,
		 									index_offset);
		MeshUtilities::GenerateTriangle(vertices + 3, indices + 3,
		 									pts2, colour.ToPremultiplied(),
		 									tex2,
		 									index_offset + 3);
		auto secondary_colour = colour;
		secondary_colour.alpha /= 4;
		MeshUtilities::GenerateLineGraph(vertices + 6, indices + 6,
										 current, next, secondary_colour.ToPremultiplied(),
										 (float) width + 1.5f,
										 Vector2f{}, Vector2f{},
										 scale,
										 index_offset + 6, offset);
		res.vertices_delta = 10;
		res.indices_delta = 12;
	}
	else {
		current_set = true;
		res.vertices_delta=0;
		res.indices_delta=0;
	}
	current = next;
	return res;
}

Rml::GraphRenderMetadata Rml::AreaGraph::GetMetadata(unsigned int input_size) {
	auto meta = GraphRenderMetadata();
	meta.vertices_delta = 6 + 4;
	meta.indices_delta = 6 + 6;
	meta.target_size = input_size - 1;
	if(current_set)
		meta.target_size += 1;
	return meta;
}
