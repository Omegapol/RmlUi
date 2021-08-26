//
// Created by Mateusz Raczynski on 4/17/2021.
//

#include "LineGraph.h"
#include "../../../Include/RmlUi/Core/GeometryUtilities.h"


Rml::GraphRenderMetadata
Rml::LineGraph::GenerateGeometryPart(Rml::Vertex *vertices, int *indices,
									 const Rml::Vector2<float> &next, int index_offset, Rml::Colourb colour,
									 Rml::Vector2f offset, Rml::Vector2f scale, double width) {
	Rml::GraphRenderMetadata res;
	if(current_set) {
		auto secondary_colour = colour;
		secondary_colour.alpha /= 4;
		GeometryUtilities::GenerateLineGraph(vertices + 4, indices + 6,
											 current, next, secondary_colour,
											 (float) width + 1.f,
											 Vector2f{}, Vector2f{},
											 scale,
											 index_offset + 4, offset);
		GeometryUtilities::GenerateLineGraph(vertices, indices,
											 current, next, colour,
											 (float) width,
											 Vector2f{}, Vector2f{},
											 scale,
											 index_offset, offset);
		res.vertices_delta=8;
		res.indices_delta=12;
	}
	else {
		current_set = true;
		res.vertices_delta=0;
		res.indices_delta=0;
	}
	current = next;
	return res;
}

Rml::GraphRenderMetadata Rml::LineGraph::GetMetadata(unsigned int input_size) {
	auto meta = GraphRenderMetadata();
	meta.vertices_delta = 8;
	meta.indices_delta = 12;
	meta.target_size = input_size - 1;
	if(current_set)
		meta.target_size += 1;
	return meta;
}

bool Rml::LineGraph::IsPointWithinElement(Rml::Vector2f point) {
	auto parent = GetParentNode();

	auto transl = parent->GetAbsoluteOffset(Box::CONTENT).Round();
	Vector2f quad_size = parent->GetBox().GetSize(Box::CONTENT).Round();
	if (point.x > transl.x && point.x < transl.x + quad_size.x &&
		point.y > transl.y && point.y < transl.y + quad_size.y) {
		auto view_point = ScreenPointToView(point, transl);

		if(!feed)
			return false;
		auto vec = this->feed->GetNeighboursXFromCache(view_point.x);
		if (vec.empty())
			return false;
		auto first = vec[0];
		auto second = vec[1];

		auto distance = distanceFromSegment(first, second, view_point);

		return distance <= 4.5f;
	}
	return false;
}

void Rml::LineGraph::GenerateGeometryInit() {
	this->current_set = false;
}

void Rml::LineGraph::GenerateGeometryStart(const Rml::Vector2<float> &from) {
	this->current_set = true;
	this->current = from;
}
