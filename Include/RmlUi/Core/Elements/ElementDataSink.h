//
// Created by Mateusz Raczynski on 4/19/2021.
//

#ifndef RMLUI_ELEMENTDATASINK_H
#define RMLUI_ELEMENTDATASINK_H

#include <RmlUi/Core/DataVariable.h>
#include <RmlUi/Core/Element.h>

namespace Rml {
	class ElementDataSink: public Element{
	public:
		RMLUI_RTTI_DefineWithParent(ElementDataSink, Element)
		using Element::Element;

		virtual bool FeedData(DataVariable data) = 0;
	};
}
#endif //RMLUI_ELEMENTDATASINK_H
