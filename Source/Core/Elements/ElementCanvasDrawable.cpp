//
// Created by Mateusz Raczynski on 4/24/2021.
//

#include "RmlUi/Core/Elements/ElementCanvasDrawable.h"

namespace Rml {
	Vector2f ElementCanvasDrawable::ScreenPointToView(const Vector2f &screen_pt, const Vector2f &transl) {
		const auto &view = GetView();
		auto view_x = view.x;
		auto view_y = view.y;
		const auto &canvas_size = GetCanvasSize();
		const auto &view_size = view_x.y - view_x.x;
		const auto &view_size_y = view_y.y - view_y.x;
		const auto &actual_x = (screen_pt.x - transl.x) / canvas_size.x * view_size + view_x.x;
		const auto &actual_y =  (canvas_size.y - (screen_pt.y - transl.y)) / canvas_size.y * view_size_y + view_y.x;
		return {actual_x, actual_y};
	}

	void ElementCanvasDrawable::SetView(CanvasView _view) {
		if(this->view.x != _view.x || this->view.y != _view.y) {
			DirtyCanvas();
			this->view = _view;
		}
	}

	CanvasView ElementCanvasDrawable::GetView() {
		return view;
	}

	Vector2f ElementCanvasDrawable::GetRatios(Vector2f canvas_size) {

		auto x_viewed = view.x;
		auto y_viewed = view.y;

		auto view_width = (x_viewed.y - x_viewed.x);
		float x_ratio = view_width / canvas_size.x;

		auto view_height = (y_viewed.y - y_viewed.x);
		if(view_height <= 0) {
			view_height = canvas_size.y;
		}
		float y_ratio = view_height / canvas_size.y;
		return {x_ratio, -y_ratio};
	}

	Vector2f ElementCanvasDrawable::GetOffset(Vector2f canvas_size) {
		const auto& ratios = GetRatios(canvas_size);
		const auto& view = GetView();
		return Vector2f{-view.x.x / ratios.x, -view.y.x / ratios.y + canvas_size.y};
	}
}