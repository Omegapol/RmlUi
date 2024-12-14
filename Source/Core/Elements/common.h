//
// Created by Mateusz Raczynski on 4/17/2021.
//

#ifndef RMLUI_COMMON_H
#define RMLUI_COMMON_H

#include <RmlUi/Core/Types.h>
#include "../../../Include/RmlUi/Config/Config.h"

namespace Rml {
	RMLUICORE_API StringList split(String text, const String &delim); //todo: move it to common funcs
	bool isUnderSegmentTowardsX(const Vector2f& line_point_a, const Vector2f& line_point_b,  const Vector2f& point);
	float distanceFromLine(const Vector2f& line_point_a, const Vector2f& line_point_b,  const Vector2f& point);
	float distanceFromSegment(const Vector2f& line_point_a, const Vector2f& line_point_b,  const Vector2f& point);
}
#endif //RMLUI_COMMON_H
