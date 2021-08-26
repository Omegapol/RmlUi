//
// Created by Mateusz Raczynski on 4/22/2021.
//

#ifndef RMLUI_GRAPHGRID_H
#define RMLUI_GRAPHGRID_H

#include "GraphScale.h"

namespace Rml {
	class GraphGrid : public GraphScale {
	public:
		GraphGrid(const String& tag);
		void GenerateGeometry(Vector2f canvasSize) override;
	};
}

#endif //RMLUI_GRAPHGRID_H
