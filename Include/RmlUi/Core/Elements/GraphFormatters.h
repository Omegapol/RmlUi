//
// Created by Mateusz Raczynski on 7/13/2021.
//

#ifndef RMLUI_GRAPHFORMATTERS_H
#define RMLUI_GRAPHFORMATTERS_H

#include <RmlUi/Core/Dictionary.h>
#include "../Header.h"

namespace Rml {


class RMLUICORE_API GraphFormatter : public NonCopyMoveable
{
public:
	using FormFunc = String(*)(double, const String&);
	static GraphFormatter* GetIstance();
	static FormFunc GetFormatter(const String& name);
	static bool SetFormatter(const String& name, FormFunc func);

private:
	UnorderedMap<String, FormFunc> formatters;
};

} // namespace Rml

#endif //RMLUI_GRAPHFORMATTERS_H
