//
// Created by Mateusz Raczynski on 7/17/2021.
//

#ifndef RMLUI_DATAGRAPHTYPES_H
#define RMLUI_DATAGRAPHTYPES_H

namespace Rml {
	struct CandleStickData {
		float high;
		float low;
		float entry;
		float exit;
		double time;
	};
}


#endif //RMLUI_DATAGRAPHTYPES_H
