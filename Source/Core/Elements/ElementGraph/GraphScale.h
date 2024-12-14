//
// Created by Mateusz Raczynski on 4/19/2021.
//

#ifndef RMLUI_GRAPHSCALE_H
#define RMLUI_GRAPHSCALE_H

#include "BaseGraph.h"

#include "RmlUi/Core/Core.h"
#include "RmlUi/Core/MeshUtilities.h"
#include "RmlUi/Core/FontEngineInterface.h"

#include "RmlUi/Core/GeometryUtilities.h"
#include "RmlUi/Core/Elements/GraphFormatters.h"

#include <cmath>

namespace Rml {
	class GraphScale : public ElementCanvasDrawable, public Element {
	protected:
		struct TexturedGeometry {
			Geometry geometry;
			Texture texture;
		};
		typedef Rml::Vector<TexturedGeometry> GeometryList;
		Geometry geometry;
		GeometryList text_geometry;
		bool geometry_dirty;
	public:
		void DirtyCanvas() override {

		}

	protected:
		int CalcSize(Vector2f view, double interval) {
			return (int) std::ceil((view.y - view.x) / interval + 2);;
		}

		void AddText(Vector2f v, String text, Colourb colour) {
			FontFaceHandle font_face_handle = GetFontFaceHandle();
			FontEffectsHandle font_effects_handle = 0;
			auto render_manager = this->GetRenderManager();
			const auto& computed = GetComputedValues();
			const TextShapingContext text_shaping_context{computed.language(), computed.direction(), computed.letter_spacing()};
			TexturedMeshList text_meshes;
			GetFontEngineInterface()->GenerateString(*render_manager,
				font_face_handle,
				font_effects_handle, text, v, colour.ToPremultiplied(), 0,
				text_shaping_context, text_meshes);
			if(text_meshes.size() > 1)
			{
				Log::Message(Log::LT_ERROR, "Too many geometry meshes generated");
			}
			TexturedGeometry geom;
			geom.geometry = render_manager->MakeGeometry(std::move(text_meshes[0].mesh));
			geom.texture = text_meshes[0].texture;
			text_geometry.emplace_back(std::move(geom));
		}

		float GetScaleInterval(Vector2f /*canvasSize*/, bool horizontal) {
			auto targetInterval = GetAttribute("interval", 10.0);
			auto targetInterval_str = GetAttribute("interval", String("auto"));

			Vector2f l_view = horizontal ? view.x : view.y;

			auto div = targetInterval;
			if (targetInterval_str == "auto") {
				auto diff = l_view.y - l_view.x;
				auto l1 = std::log10(diff / 2);
				auto l2 = std::ceil(l1);
				div = std::pow(10, l2 - 1);
			}
			return static_cast<float>(div);
		}

	public:
		GraphScale(const String &tag) : Element(tag) {};

		Vector2f GetCanvasSize() override {
			auto parent = GetParentNode();
			Vector2f quad_size = parent->GetBox().GetSize(BoxArea::Content).Round();
			return quad_size;
		}

		void RenderOnCanvas(Vector2f /*canvas_size*/, Vector2f transl) override {
			geometry.Render(transl);

			for (size_t i = 0; i < text_geometry.size(); ++i)
				text_geometry[i].geometry.Render(transl, text_geometry[i].texture);
		}

		void GenerateGeometry(Vector2f canvasSize) override {
			// Release the old geometry before specifying the new vertices.
			geometry.Release(Geometry::ReleaseMode::ClearMesh);
			for (size_t i = 0; i < text_geometry.size(); ++i)
				text_geometry[i].geometry.Release(Geometry::ReleaseMode::ClearMesh);

			auto man = this->GetRenderManager();
			auto mesh = Mesh();

			Vector <Vertex> &vertices = mesh.vertices;
			Vector<int> &indices = mesh.indices;

			auto isHorizontal = GetAttribute("horizontal", String("true")) == "true";

			auto view_x = view.x;
			auto view_y = view.y;
			auto l_view = view_x;
			auto view_width = (l_view.y - l_view.x);
			auto ratios = GetRatios(canvasSize);
			auto canvas_size = canvasSize.x;
			if (!isHorizontal) {
				//todo: using canvas size, but needs to be replaced with actual y view value
				view_width = view.y.y - view.y.x;
				l_view = view.y;
				canvas_size = canvasSize.y;
			}

			auto div = GetScaleInterval(canvasSize, isHorizontal);

			float start_x = isHorizontal ? (int) (view.x.x / div) * div : (int) (view.y.x / div) * div;

			if (start_x <= 0)
				start_x -= div;

			if (view_width <= 0 || canvasSize.x <= 0 || canvasSize.y <= 0)
				return;
			auto size = CalcSize({start_x, l_view.y}, div) + 1;

			if (size <= 0)
				return;

			vertices.resize(size * 4);
			indices.resize(size * 6);
			text_geometry.reserve(size);

			const ComputedValues &computed = GetComputedValues();

			//todo: width computation duplicated
			auto width = 2.0;
			auto computed_width = computed.width();
			if (computed_width.type == Style::Width::Length && computed_width.value != 0) {
				width = computed_width.value;
			}

			Colourb colour = computed.color();
			colour.alpha = (byte)(computed.opacity() * (float) colour.alpha);
			ColourbPremultiplied colour_premultiplied = colour.ToPremultiplied();
			auto scale = Vector2f{ratios.x, ratios.y};

			auto v1 = Vector2f{0, 10};
			auto v2 = Vector2f{canvasSize.x, 10};
			if (!isHorizontal) {
				v1 = {v1.y, v1.x};
				v2 = {v2.y, canvasSize.y};
			}
			v1.y -= canvasSize.y;
			v2.y -= canvasSize.y;

			auto idx = 0;
			Rml::MeshUtilities::GenerateLineGraph(&vertices[0] + idx * 4, &indices[0] + idx * 6,
												 v1,
												 v2,
												 colour_premultiplied,
												 (float) width,
												 Vector2f{}, Vector2f{},
												 {1.0f, -1.0f},
												 idx * 4,
												 {});

			auto offset2 = GetOffset(canvasSize);

			struct ScaleLabel {
				String text;
				float position;
			};

			auto formatter = GraphFormatter::GetFormatter(
					GetAttribute("formatter", String("default"))
			);
			auto format = GetAttribute("format", String("%.1f"));
			Vector <ScaleLabel> labels;
			for (auto d = start_x; d < l_view.y + div; d += div) {
				auto text = formatter(d, format);
				ScaleLabel val = ScaleLabel{text, d};
				val.position = d;
				labels.push_back(val);
			}

			FontFaceHandle font_face_handle = GetFontFaceHandle();
			const TextShapingContext text_shaping_context{computed.language(), computed.direction(), computed.letter_spacing()};
			auto text_height = GetLineHeight();
			auto text_width = 0;
			for (auto &label: labels) {
				text_width = std::max(text_width, GetFontEngineInterface()->GetStringWidth(font_face_handle, label.text, text_shaping_context));
			 }

			double min_position = GetAttribute("left-margin", -0.5) * canvas_size;
			double max_position = (1.0 - GetAttribute("right-margin", -0.5)) * canvas_size;
			//ignoring text_height because it is alright to have text slightly off screen
			auto text_size = isHorizontal ? text_width : 0;

			{
				auto l2 = isHorizontal ? canvasSize.x / (1.05 * text_width) : canvasSize.y / text_height;
				auto l3 = std::ceil(labels.size() / l2);
				auto last_text = String("");
				for (auto &label: labels) {
					auto index = (label.position / div);
					auto is_displayed = std::fmod(index, l3) == 0;
					auto d = label.position;
					is_displayed = is_displayed && last_text != label.text;
					last_text = label.text;

					float target_pos = 0.f;
					if (!isHorizontal) {
						target_pos = d / scale.y + offset2.y;
					} else {
						target_pos = d / scale.x + offset2.x;
					}

					// allow user to specify % of graph that doesn't have labels generated
					is_displayed = is_displayed && target_pos >= min_position;
					is_displayed = is_displayed && target_pos + text_size <= max_position;

					auto label_offset = is_displayed ? 4.f : 0.f;
					if (!isHorizontal) {
						v1 = {3, target_pos};
						v2 = {13 + label_offset, target_pos};
					} else {
						v1 = {target_pos, -5 + canvasSize.y};
						v2 = {target_pos, -15 - label_offset + canvasSize.y};
					}
					auto text_screen_pos = Vector2f{v2.x, v2.y};

					auto line_width = width;
					if (is_displayed) {
						AddText(text_screen_pos, label.text, colour);
						line_width *= 1.4;
					}

					idx += 1;
					MeshUtilities::GenerateLineGraph(&vertices[0] + idx * 4, &indices[0] + idx * 6,
														 v1,
														 v2,
														 colour_premultiplied,
														 (float) line_width,
														 Vector2f{}, Vector2f{},
														 {1.0f, 1.0f},
														 idx * 4,
														 {});
				}
			}
			geometry = man->MakeGeometry(std::move(mesh));
		}
	};
}

#endif //RMLUI_GRAPHSCALE_H
