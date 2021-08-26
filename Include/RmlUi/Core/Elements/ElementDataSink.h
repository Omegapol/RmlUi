//
// Created by Mateusz Raczynski on 4/19/2021.
//

#ifndef RMLUI_ELEMENTDATASINK_H
#define RMLUI_ELEMENTDATASINK_H


namespace Rml {
	class ElementDataSink {
	public:
		virtual bool FeedData(DataVariable data) = 0;
	};
}
#endif //RMLUI_ELEMENTDATASINK_H
