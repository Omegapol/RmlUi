/*
 * This source file is part of RmlUi, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://github.com/mikke89/RmlUi
 *
 * Copyright (c) 2008-2010 CodePoint Ltd, Shift Technology Ltd
 * Copyright (c) 2019 The RmlUi Team, and contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "../../../Include/RmlUi/Core/Elements/ElementCanvas.h"
#include "../../../Include/RmlUi/Core/URL.h"
#include "ElementGraph/LineGraph.h"
#include <RmlUi/Core/Traits.h>
#include <limits>

namespace Rml {

// Constructs a new ElementCanvas.
	ElementCanvas::ElementCanvas(const String &tag) : Element(tag), dimensions(-1, -1), rect_source(RectSource::None) {
		dimensions_scale = 1.0f;
		geometry_dirty = false;
		texture_dirty = true;
		AddEventListener(EventId::Mousedown, this, true);
		AddEventListener(EventId::Mouseup, this, true);
		AddEventListener(EventId::Mousemove, this, true);
		AddEventListener(EventId::Mouseout, this, true);
		AddEventListener(EventId::Mousescroll, this, true);
		entry_mouse = {-1, -1};

		view_lock.x.x = -std::numeric_limits<float>::max();
		view_lock.y.x = -std::numeric_limits<float>::max();
		view_lock.x.y = std::numeric_limits<float>::max();
		view_lock.y.y = std::numeric_limits<float>::max();
		zoom_lock_x.x = -std::numeric_limits<float>::max();
		zoom_lock_y.x = -std::numeric_limits<float>::max();
		zoom_lock_x.y = std::numeric_limits<float>::max();
		zoom_lock_y.y = std::numeric_limits<float>::max();
	}

	ElementCanvas::~ElementCanvas() {
	}

// Sizes the box to the element's inherent size.
	bool ElementCanvas::GetIntrinsicDimensions(Vector2f &_dimensions, float &_ratio) {
		// Check if we need to reload the texture.
		if (texture_dirty)
			LoadTexture();

		// Calculate the x dimension.
		if (HasAttribute("width"))
			dimensions.x = GetAttribute<float>("width", -1);
		else if (rect_source == RectSource::None)
			dimensions.x = (float) texture.GetDimensions().x;
		else
			dimensions.x = rect.Width();

		// Calculate the y dimension.
		if (HasAttribute("height"))
			dimensions.y = GetAttribute<float>("height", -1);
		else if (rect_source == RectSource::None)
			dimensions.y = (float) texture.GetDimensions().y;
		else
			dimensions.y = rect.Height();

		dimensions *= dimensions_scale;

		// Return the calculated dimensions. If this changes the size of the element, it will result in
		// a call to 'onresize' below which will regenerate the geometry.
		_dimensions = dimensions;
		_ratio = dimensions.x / dimensions.y;

		return true;
	}

// Renders the element.
	void ElementCanvas::OnRender() {
		auto transl = GetAbsoluteOffset(BoxArea::Content).Round();
		Vector2f quad_size = GetBox().GetSize(BoxArea::Content).Round();

		if (geometry_dirty)
			GenerateGeometry();

		GetRenderInterface()->EnableScissorRegion(true);
		Rectanglei clip_rect;
		ElementUtilities::GetClippingRegion(this, clip_rect, {}, true);
		GetRenderInterface()->SetScissorRegion(clip_rect);

		for (auto i = 0; i < GetNumChildren(true); ++i) {
			auto child = GetChild(i);
			auto ptr = rmlui_dynamic_cast<ElementCanvasDrawable *>(child);
			if (ptr) {
				ptr->RenderOnCanvas(quad_size, transl);
			}
		}
	}

// Called when attributes on the element are changed.
	void ElementCanvas::OnAttributeChange(const ElementAttributes &changed_attributes) {
		// Call through to the base element's OnAttributeChange().
		Element::OnAttributeChange(changed_attributes);

		bool dirty_layout = false;

		// Check for a changed 'src' attribute. If this changes, the old texture handle is released,
		// forcing a reload when the layout is regenerated.
		if (changed_attributes.find("src") != changed_attributes.end() ||
			changed_attributes.find("sprite") != changed_attributes.end()) {
			texture_dirty = true;
			dirty_layout = true;
		}

		// Check for a changed 'width' attribute. If this changes, a layout is forced which will
		// recalculate the dimensions.
		if (changed_attributes.find("width") != changed_attributes.end() ||
			changed_attributes.find("height") != changed_attributes.end()) {
			dirty_layout = true;
		}

		// Check for a change to the 'rect' attribute. If this changes, the coordinates are
		// recomputed and a layout forced. If a sprite is set to source, then that will override any attribute.
		if (changed_attributes.find("rect") != changed_attributes.end()) {
			UpdateRect();

			// Rectangle has changed; this will most likely result in a size change, so we need to force a layout.
			dirty_layout = true;
		}

		auto staticview = changed_attributes.find("view-x");
		if (staticview != changed_attributes.end()) {
			auto changed = ParseViewStr(staticview->second.Get<String>(), view.x);
			if (changed)
				geometry_dirty = true;
		}

		staticview = changed_attributes.find("view-y");
		if (staticview != changed_attributes.end()) {
			auto changed = ParseViewStr(staticview->second.Get<String>(), view.y);
			if (changed)
				geometry_dirty = true;
		}

		bool adjust_cam = false;

		auto parse_view_lock = [this, &changed_attributes, &adjust_cam](const String &name, Vector2f &storage) {
			auto view_lock_set = changed_attributes.find(name);
			if (view_lock_set != changed_attributes.end()) {
				auto changed = ParseViewStr(view_lock_set->second.Get<String>(), storage);
				if (storage.x == storage.y) {
					storage.x = -std::numeric_limits<float>::max();
					storage.y = std::numeric_limits<float>::max();
				}
				if (changed) {
					geometry_dirty = true;
					adjust_cam = true;
				}
			}
		};
		parse_view_lock("view-x-lock", view_lock.x);
		parse_view_lock("view-y-lock", view_lock.y);

		auto parse_zoom_lock = [this, &changed_attributes, &adjust_cam](const String &name, Vector2f &storage) {
			auto view_lock_set = changed_attributes.find(name);
			if (view_lock_set != changed_attributes.end()) {
				auto changed = ParseViewStr(view_lock_set->second.Get<String>(), storage);
				if (storage.x == -1) {
					storage.x = -std::numeric_limits<float>::max();
				}
				if (storage.y == -1) {
					storage.y = std::numeric_limits<float>::max();
				}
				if (changed) {
					geometry_dirty = true;
					adjust_cam = true;
				}
			}
		};
		parse_zoom_lock("zoom-x-lock", zoom_lock_x);
		parse_zoom_lock("zoom-y-lock", zoom_lock_y);

		{
			auto zoom_attr = changed_attributes.find("zoom-min");
			if (zoom_attr != changed_attributes.end()) {
				auto value = zoom_attr->second.Get<float>();
				zoom_lock_x.x = value;
				zoom_lock_y.x = value;
				adjust_cam = true;
			}
		}
		{
			auto zoom_attr = changed_attributes.find("zoom-max");
			if (zoom_attr != changed_attributes.end()) {
				auto value = zoom_attr->second.Get<float>();
				zoom_lock_x.y = value;
				zoom_lock_y.y = value;
				adjust_cam = true;
			}
		}

		if(adjust_cam)
		{
			Zoom(0.0f);
		}

		auto startview = changed_attributes.find("start-view");
		if (startview != changed_attributes.end()) {
			Vector2f v;
			auto changed = ParseViewStr(startview->second.Get<String>(), v);
			if (!HasAttribute("static-view") && changed) {
				Vector2f quad_size = GetBox().GetSize(BoxArea::Content).Round();
				geometry_dirty = true;
				view = {v, {0, quad_size.y}};
			}
		}

		if (dirty_layout)
			DirtyLayout();
	}

	void ElementCanvas::OnPropertyChange(const PropertyIdSet &changed_properties) {
		Element::OnPropertyChange(changed_properties);

		if (changed_properties.Contains(PropertyId::ImageColor) ||
			changed_properties.Contains(PropertyId::Opacity)) {
			GenerateGeometry();
		}
	}

	void ElementCanvas::OnChildAdd(Element *child) {
		if (child == this && texture_dirty) {
			// Load the texture once we have attached to the document
			LoadTexture();
		}
	}

// Regenerates the element's geometry.
	void ElementCanvas::OnResize() {
		GenerateGeometry();
	}

	void ElementCanvas::OnDpRatioChange() {
		texture_dirty = true;
		DirtyLayout();
	}

	void ElementCanvas::OnStyleSheetChange() {
		if (HasAttribute("sprite")) {
			texture_dirty = true;
			DirtyLayout();
		}
	}

	void ElementCanvas::GenerateGeometry() {
		Vector2f quad_size = GetBox().GetSize(BoxArea::Content).Round();

		for (auto i = 0; i < GetNumChildren(true); ++i) {
			auto child = GetChild(i);
			auto ptr = rmlui_dynamic_cast<ElementCanvasDrawable *>(child);
			if (ptr) {
				ptr->SetView(view);
				ptr->GenerateGeometry(quad_size);
			}
		}

		geometry_dirty = false;
	}

	bool ElementCanvas::LoadTexture() {
		texture_dirty = false;
		geometry_dirty = true;
		dimensions_scale = 1.0f;

		auto man = this->GetRenderManager();
		const float dp_ratio = ElementUtilities::GetDensityIndependentPixelRatio(this);

		// Check for a sprite first, this takes precedence.
		const String sprite_name = GetAttribute<String>("sprite", "");
		if (!sprite_name.empty()) {
			// Load sprite.
			bool valid_sprite = false;

			if (ElementDocument * document = GetOwnerDocument()) {
				if (const StyleSheet *style_sheet = document->GetStyleSheet()) {
					if (const Sprite *sprite = style_sheet->GetSprite(sprite_name)) {
						rect = sprite->rectangle;
						rect_source = RectSource::Sprite;
						texture = sprite->sprite_sheet->texture_source.GetTexture(*man);
						dimensions_scale = sprite->sprite_sheet->display_scale * dp_ratio;
						valid_sprite = true;
					}
				}
			}

			if (!valid_sprite) {
				texture = Texture();
				rect_source = RectSource::None;
				UpdateRect();
				Log::Message(Log::LT_WARNING, "Could not find sprite '%s' specified in img element %s",
							 sprite_name.c_str(), GetAddress().c_str());
				return false;
			}
		} else {
			// Load image from source URL.
			const String source_name = GetAttribute<String>("src", "");
			if (source_name.empty()) {
				texture = Texture();
				rect_source = RectSource::None;
				return false;
			}

			URL source_url;

			if (ElementDocument * document = GetOwnerDocument())
				source_url.SetURL(document->GetSourceURL());

			texture = man->LoadTexture(source_name, source_url.GetPath());
			dimensions_scale = dp_ratio;
		}

		return true;
	}

	void ElementCanvas::UpdateRect() {
		if (rect_source != RectSource::Sprite) {
			bool valid_rect = false;

			String rect_string = GetAttribute<String>("rect", "");
			if (!rect_string.empty()) {
				StringList coords_list;
				StringUtilities::ExpandString(coords_list, rect_string, ' ');

				if (coords_list.size() != 4) {
					Log::Message(Log::LT_WARNING,
								 "Element '%s' has an invalid 'rect' attribute; rect requires 4 space-separated values, found %zu.",
								 GetAddress().c_str(), coords_list.size());
				} else {
					rect.p0 = {static_cast<float>(std::atof(coords_list[0].c_str())), static_cast<float>(std::atof(coords_list[1].c_str()))};
					rect.p1 = {static_cast<float>(rect.p0.x + std::atof(coords_list[2].c_str())),
						static_cast<float>(rect.p0.y + std::atof(coords_list[3].c_str()))};

					// We have new, valid coordinates; force the geometry to be regenerated.
					valid_rect = true;
					geometry_dirty = true;
					rect_source = RectSource::Attribute;
				}
			}

			if (!valid_rect) {
				rect = {};
				rect_source = RectSource::None;
			}
		}
	}

	void ElementCanvas::ProcessEvent(Event &event) {
		unsigned int i = 0;
		++i;
		if (event.GetId() == EventId::Mousedown) {
			entry_mouse = event.GetUnprojectedMouseScreenPos();
		}
		if (event.GetId() == EventId::Mouseup) {
			entry_mouse = {-1, -1};
		}
		if (event.GetId() == EventId::Mouseout) {
			auto mouse = event.GetUnprojectedMouseScreenPos();
			if (!IsPointWithinElement(mouse))
				entry_mouse = {-1, -1};
		}
		if (event.GetId() == EventId::Mousemove) {
			auto mouse = event.GetUnprojectedMouseScreenPos();
			last_mouse_view_pos = ScreenToView(mouse);
		}
		if (event.GetId() == EventId::Mousemove && entry_mouse.x != -1) {
			auto start = ScreenToView(entry_mouse);
			auto end_mouse = event.GetUnprojectedMouseScreenPos();
			auto end = ScreenToView(end_mouse);
			auto delta = end - start;

			MoveView(delta);

			entry_mouse = end_mouse;
			geometry_dirty = true;
		}
		if (event.GetId() == EventId::Mousescroll) {
			auto params = event.GetParameters();
			auto delta = params["wheel_delta_y"].Get<float>() * 5;
			Zoom(delta);

			geometry_dirty = true;
			event.StopPropagation();
		}
	}

	bool ElementCanvas::ParseViewStr(const String& s, Vector2f &res) {
		if (s != "") {
			auto arr = split(s, ",");
			//todo: validate arr
			//todo: validate numbers
			float x,y;
			auto func = [](float& res, const String& value, float default_val){
				if(value.empty()) {
					res = default_val;
					return;
				}
				res = static_cast<float>(std::atof(value.c_str()));
			};
			func(x, arr[0], -std::numeric_limits<float>::max());
			func(y, arr[1], std::numeric_limits<float>::max());
			res = {x, y};
			return true;
		}
		return false;
	}

	Vector2f ElementCanvas::ScreenToView(const Vector2f &screen_pos) {
		auto transl = GetAbsoluteOffset(BoxArea::Content).Round();
		Vector2f quad_size = GetBox().GetSize(BoxArea::Content).Round();

		const auto &view_size_x = view.x.y - view.x.x;
		const auto &view_size_y = view.y.y - view.y.x;
		const auto &actual_x = (screen_pos.x - transl.x) / quad_size.x * view_size_x + view.x.x;
		const auto &actual_y = (quad_size.y - (screen_pos.y - transl.y)) / quad_size.y * view_size_y + view.y.x;
		return {actual_x, actual_y};
	}

	Vector2f ElementCanvas::ViewToScreen(const Vector2f &view_pos) {
		//todo: test this
		auto transl = GetAbsoluteOffset(BoxArea::Content).Round();
		Vector2f quad_size = GetBox().GetSize(BoxArea::Content).Round();

		const auto &view_size_x = view.x.y - view.x.x;
		const auto &view_size_y = view.y.y - view.y.x;
		const auto &actual_x = view_pos.x;
		const auto &actual_y = view_pos.y;
		Vector2f screen_pos;
		screen_pos.x = (actual_x - view.x.x) / view_size_x * quad_size.x + transl.x;
		screen_pos.y = transl.y + quad_size.y - (actual_y - view.y.x) / view_size_y * quad_size.y;
		return screen_pos;
	}

	void ElementCanvas::GetZoomConstraints(Vector2f &_zoom_lock_x, Vector2f &_zoom_lock_y) const {
		_zoom_lock_x = this->zoom_lock_x;
		_zoom_lock_y = this->zoom_lock_y;
	}

	void ElementCanvas::SetZoomConstraints(Vector2f _zoom_lock_x, Vector2f _zoom_lock_y) {
		this->zoom_lock_x = _zoom_lock_x;
		this->zoom_lock_y = _zoom_lock_y;
		Zoom(0.0f);
	}

	CanvasView ElementCanvas::GetLockView() const {
		return view_lock;
	}

	void ElementCanvas::LockView(CanvasView cv) {
		view_lock = cv;
		Zoom(0.0f);
	}

	CanvasView ElementCanvas::GetView() const {
		return view;
	}

	void ElementCanvas::SetView(CanvasView _view) {
		view = _view;
	}

	void ElementCanvas::MoveView(Vector2f delta) {
		CanvasView tmp = view;
		auto calc_move = [](Vector2f &cur_pos, Vector2f locked, float delta) {
			auto target_pos = cur_pos - Vector2f{delta, delta};
			if (target_pos.x > locked.x && target_pos.y < locked.y)
				cur_pos = target_pos;
		};
		calc_move(tmp.x, view_lock.x, delta.x);
		calc_move(tmp.y, view_lock.y, delta.y);

		SetView(tmp);
	}

	void ElementCanvas::Zoom(float delta) {
		Vector2f view_x = this->view.x;
		Vector2f view_y = this->view.y;
		auto calc = [delta](Vector2f &_view, Vector2f lock, Vector2f zoom_lock, float mouse_view_pos) {
			auto scale = 1.0f + std::abs(delta) / 100.f;
			if (delta < 0)
				scale = 1 / scale;
			auto mid = (_view.x + _view.y) / 2.f;

			auto width = _view.y - _view.x;
			auto ratio = (mouse_view_pos - _view.x) / width;
			auto dmove = (width) * scale - width;
			auto right = dmove * (1 - ratio) + width / 2;
			auto left = dmove * ratio + width / 2;

			if (left + right < zoom_lock.x || left + right > zoom_lock.y)
				return;
			auto res = Vector2f{mid - left, mid + right};

			if (lock.x > res.x) {
				auto lock_delta = lock.x - res.x;
				res += Vector2f{lock_delta, lock_delta};
				if (lock.y < res.y) {
					res.y = lock.y;
				}
			}
			if (lock.y < res.y) {
				auto lock_delta = lock.y - res.y;
				res += Vector2f{lock_delta, lock_delta};
				if (lock.x > res.x) {
					res.x = lock.x;
				}
			}

			_view = res;
		};
		calc(view_x, view_lock.x, zoom_lock_x, last_mouse_view_pos.x);
		calc(view_y, view_lock.y, zoom_lock_y, last_mouse_view_pos.y);

		this->view.x = view_x;
		this->view.y = view_y;
		geometry_dirty = true;
	}


} // namespace Rml
