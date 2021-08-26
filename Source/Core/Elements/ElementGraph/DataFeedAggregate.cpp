//
// Created by Mateusz Raczynski on 7/10/2021.
//

#include "DataFeedAggregate.h"

namespace Rml {

	void Div(Vector2f &obj, float div)
	{
		obj.y /= div;
	}

	template<>
	void SetX(CandleStickData& obj, double x)
	{
		obj.time = x;
	}

	template<>
	inline double GetX(CandleStickData el) {
		return el.time;
	}
}