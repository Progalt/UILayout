
#ifndef LATTE_COMPONENT_EVENTS_H
#define LATTE_COMPONENT_EVENTS_H

#include "../OS/Event.h"

typedef struct LatteNode LatteNode;

namespace latte
{
	bool handleNodeEvent(Event evnt, LatteNode* node);
}

#endif // LATTE_COMPONENT_EVENTS_H