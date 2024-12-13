//
// Created by Mateusz Raczynski on 4/22/2021.
//

#include "GraphGrid.h"

void Rml::GraphGrid::GenerateGeometry(Rml::Vector2f canvasSize) {

	// Release the old geometry before specifying the new vertices.
	geometry.Release(Geometry::ReleaseMode::ClearMesh);

	auto man = this->GetRenderManager();
	auto mesh = Mesh();

	Vector <Vertex> &vertices = mesh.vertices;
	Vector<int> &indices = mesh.indices;

	auto l_view = view.x;
	auto view_width = (l_view.y - l_view.x);

	if (view_width <= 0 || canvasSize.x <= 0 || canvasSize.y <= 0)
		return;

	auto div_x = GetScaleInterval(canvasSize, true);
	auto div_y = GetScaleInterval(canvasSize, false);
	auto size = CalcSize(l_view, div_x);

	auto r_view = view.y;
	size += CalcSize(r_view, div_y);

	vertices.resize(size*4);
	indices.resize(size*6);

	const ComputedValues& computed = GetComputedValues();

	//todo: width computation duplicated
	auto width = 2.f;
	auto computed_width = computed.width();
	if(computed_width.type == Style::Width::Length && computed_width.value != 0)
	{
		width = computed_width.value;
	}

	Colourb colour = computed.color();
	ColourbPremultiplied colour_premultiplied = ColourbPremultiplied(colour.red, colour.green, colour.red, (computed.opacity() * (float) colour.alpha));
	colour.alpha = (byte) (computed.opacity() * (float) colour.alpha);
	auto scale = Vector2f{view_width / canvasSize.x, -1.0f};
	scale = GetRatios(canvasSize);

	auto idx = 0;

	auto offset2 = GetOffset(canvasSize);

	for(auto d = (int) (l_view.x / div_x) * div_x; d < l_view.y + div_x ; d+=div_x)
	{
		Vector2f v1 = {d / scale.x + offset2.x, 0};
		Vector2f v2 = {d  / scale.x + offset2.x, canvasSize.y};

		MeshUtilities::GenerateLineGraph(&vertices[0] + idx*4, &indices[0] + idx*6,
											 v1,
											 v2,
											 colour_premultiplied,
											 (float) width,
											 Vector2f{}, Vector2f{},
											 {1.0f, 1.0f},
											 idx*4,
											 {});
		idx += 1;
	}


	for(auto d = (int) (r_view.x / div_y) * div_y; d < r_view.y + div_y ; d+=div_y)
	{
		Vector2f v1 = {0, d / scale.y + offset2.y};
		Vector2f v2 = {canvasSize.x, d / scale.y + offset2.y};

		MeshUtilities::GenerateLineGraph(&vertices[0] + idx*4, &indices[0] + idx*6,
											 v1,
											 v2,
											 colour_premultiplied,
											 (float) width,
											 Vector2f{}, Vector2f{},
											 {1.0f, 1.0f},
											 idx*4,
											 {});
		idx += 1;
	}
	geometry = man->MakeGeometry(std::move(mesh));
//	Log::Message(Log::LT_DEBUG, "Size: %d Actual: %d", size, idx);
}

Rml::GraphGrid::GraphGrid(const Rml::String &tag) : GraphScale(tag) {

}
