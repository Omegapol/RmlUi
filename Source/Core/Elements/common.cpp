//
// Created by Mateusz Raczynski on 4/17/2021.
//

#include "common.h"

namespace Rml {
	StringList split(String text, const String &delim) //todo: move it to common funcs
	{
		StringList list{};

		size_t pos = 0;
		std::string token;
		while ((pos = text.find(delim)) != std::string::npos) {
			token = text.substr(0, pos);
			list.push_back(token);
			text.erase(0, pos + delim.length());
		}
		list.push_back(text);
		return list;
	}

	float distanceFromLine(const Vector2f& line_point_a, const Vector2f& line_point_b,  const Vector2f& point)
	{

		auto A = line_point_a.y - line_point_b.y;
		auto B = line_point_b.x - line_point_a.x;
		auto C = line_point_a.x * line_point_b.y - line_point_b.x * line_point_a.y;
		auto distance = std::abs(A*point.x + B*point.y + C) / std::sqrt(A*A + B*B);
		return distance;
	}

	float distanceFromSegment(const Vector2f& line_point_a, const Vector2f& line_point_b,  const Vector2f& point)
	{
		const auto line_vector = line_point_b - line_point_a;
		auto lv_length2 = static_cast<float>(std::pow(line_vector.Magnitude(), 2));
		if(lv_length2 == 0)
			return (point - line_point_a).Magnitude();

		float raw = (point - line_point_a).DotProduct(line_vector) / lv_length2;
		const auto t = std::max(0.f, std::min(1.f, raw));
		const auto projection = line_point_a + t * line_vector;
		return (point - projection).Magnitude();
	}

	float cross2d(const Vector2f& a, const Vector2f& b)
	{
		return a.x*b.y - a.y*b.x;
	}

	bool isUnderSegmentTowardsX(const Vector2f& line_point_a, const Vector2f& line_point_b,  const Vector2f& point)
	{
		auto dir = (line_point_b-line_point_a).Normalise();
		auto b = (point.x - line_point_a.x)/dir.x;
		auto projected = b*dir + line_point_a;

		bool expect_below = projected.y > 0.f;
		auto ab = line_point_b - line_point_a;
		auto ap = point - line_point_a;
		auto is_negative_cross = cross2d(ab, ap) < 0;
		auto pr = point.y < 0.0f;
		auto res = (!pr && expect_below && is_negative_cross) || (pr && !expect_below && !is_negative_cross);
		return res;
	}
}
