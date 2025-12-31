
#ifndef LATTE_COMPONENT_EVENTS_H
#define LATTE_COMPONENT_EVENTS_H

#include "../OS/Event.h"
#include <sol/sol.hpp>

typedef struct LatteNode LatteNode;

namespace latte
{
	bool handleNodeEvent(Event evnt, LatteNode* node, sol::state_view luaState);
}

#endif // LATTE_COMPONENT_EVENTS_H