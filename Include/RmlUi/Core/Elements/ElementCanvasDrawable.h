//
// Created by Mateusz Raczynski on 4/19/2021.
//

#ifndef RMLUI_ELEMENTCANVASDRAWABLE_H
#define RMLUI_ELEMENTCANVASDRAWABLE_H

#include "ElementDataSink.h"
#include <RmlUi/Core/Types.h>

namespace Rml {
	struct CanvasView {
		Vector2f x;
		Vector2f y;
	};

	class RMLUICORE_API ElementCanvasDrawable: public ElementDataSink {
	protected:
		CanvasView view;
	public:
		RMLUI_RTTI_DefineWithParent(ElementCanvasDrawable, ElementDataSink)
		using ElementDataSink::ElementDataSink;

		virtual Vector2f ScreenPointToView(const Vector2f& screen_pt, const Vector2f& transl);

		virtual void RenderOnCanvas(Vector2f canvas_size, Vector2f transl) = 0;
		virtual void GenerateGeometry(Vector2f canvasSize) = 0;

		Vector2f GetRatios(Vector2f canvas_size);
		Vector2f GetOffset(Vector2f canvas_size);
		virtual Vector2f GetCanvasSize() = 0;
		virtual void SetView(CanvasView view);
		virtual CanvasView GetView();

		virtual void DirtyCanvas() = 0;
	};
}

#endif //RMLUI_ELEMENTCANVASDRAWABLE_H
