//
// Created by Mateusz Raczynski on 7/13/2021.
//

#include "../../../Include/RmlUi/Core/Elements/GraphFormatters.h"
#include <RmlUi/Core/Core.h>
#include "../../../Include/RmlUi/Core/SystemInterface.h"

#include <ctime>
#include <cmath>
#include <chrono>

static std::tm convert_time(std::time_t time_t)
{
	std::tm res;
#if defined(WIN32) || defined(__MINGW32__)  || defined(__MINGW64__)
	gmtime_s(&res, &time_t);
#else
	gmtime_r(&time_t, &res);
#endif
	return res;
}

static std::tm get_time(double x)
{
	auto hours = (int) x / 3600;
	auto seconds_left = std::fmod(x, 3600);
	auto minutes = seconds_left / 60;
	auto seconds = std::fmod(seconds_left, 60);
	auto days = hours / 24;
	hours = hours % 24;
	auto years = days / 365;
	days = days % 365;
	auto months = std::floor(days / 31);

	std::tm res{};
	res.tm_hour = hours;
	res.tm_min = static_cast<int>(minutes);
	res.tm_sec = static_cast<int>(seconds);
	res.tm_yday = days;
	res.tm_mon = static_cast<int>(months);
	res.tm_year = years;

	res.tm_mday = 1;
	res.tm_wday = 1;
	res.tm_isdst = false;
	return res;
}

namespace Rml {
	static GraphFormatter *instance = nullptr;

	thread_local char buf[1024];

	static String DefaultFormatter(double x, const String& args) {
		auto s = std::to_string(x);
		snprintf(buf, 1024, args.c_str(), x);
		return String(buf);
	}

	static String DateFormatter(double x, const String& args) {
		auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::time_point(
				std::chrono::duration_cast<std::chrono::seconds>(std::chrono::duration<double>(x))));
		std::tm res = convert_time(time);

		std::strftime(buf, 1024, args.c_str(), &res);
		return String(buf);
	}

	static String SinceStartDateFormatter(double x, const String& args) {
		auto dt_since_start = Rml::GetSystemInterface()->GetElapsedTime();
		auto target_ts = std::time(nullptr) - std::floor(dt_since_start) + std::floor(x);
		std::tm res = convert_time(static_cast<time_t>(target_ts));

		std::strftime(buf, 1024, args.c_str(), &res);
		return String(buf);
	}

	static String DurationFormatter(double x, const String& args) {
		auto res = get_time(std::abs(x));

		auto arg = args;

		if(arg == "auto")
		{
			if(res.tm_year != 0)
			{
				arg = "%Yy %Mm";
			}
			else if(res.tm_mon != 0)
			{
				arg = "%Mm %jd";
			}
			else if(res.tm_yday != 0)
			{
				arg = "%jd %Hh";
			}
			else if(res.tm_hour != 0)
			{
				arg = "%Hh:%Mm";
			}
			else
			{
				arg = "%Mm:%Ss";
			}
			arg = String("%s") + arg;
		}
		else if(arg == "%D")
			arg = "%m/%d/%y";
		else if(arg == "%F")
			arg = "%Y-%m-%d";
		else if(arg == "%T")
			arg = "%H:%M:%S";

		std::string hour = std::to_string(res.tm_hour);
		hour = res.tm_hour < 10 ? "0" + hour : hour;
		std::string min = std::to_string(res.tm_min);
		min = res.tm_min < 10 ? "0" + min : min;
		std::string sec = std::to_string(res.tm_sec);
		sec = res.tm_sec < 10 ? "0" + sec : sec;

		arg = StringUtilities::Replace(arg, "%Y", std::to_string(res.tm_year));
		arg = StringUtilities::Replace(arg, "%y", std::to_string(res.tm_year % 100));
		arg = StringUtilities::Replace(arg, "%m", std::to_string(res.tm_mon));
		arg = StringUtilities::Replace(arg, "%j", std::to_string(res.tm_yday));
		arg = StringUtilities::Replace(arg, "%H", hour);
		arg = StringUtilities::Replace(arg, "%M", min);
		arg = StringUtilities::Replace(arg, "%S", sec);
		arg = StringUtilities::Replace(arg, "%s", x < 0? "-": "");

		return arg;
	}

	GraphFormatter::FormFunc GraphFormatter::GetFormatter(const String &name) {
		UnorderedMap <String, FormFunc> &dict = GetIstance()->formatters;
		auto res = dict.find(name);
		if (res == dict.end() || res->second == nullptr)
			return &DefaultFormatter;
		return res->second;
	}

	bool GraphFormatter::SetFormatter(const String &name, GraphFormatter::FormFunc func) {
		if (GetFormatter(name) != &DefaultFormatter)
			return false;
		GetIstance()->formatters[name] = func;
		return true;
	}

	GraphFormatter *GraphFormatter::GetIstance() {
		if (instance == nullptr) {

			//todo: memory leak
			instance = new GraphFormatter();
			instance->formatters["date"] = &DateFormatter;
			instance->formatters["date_st"] = &SinceStartDateFormatter;
			instance->formatters["duration"] = &DurationFormatter;
		}
		return instance;
	}
}
