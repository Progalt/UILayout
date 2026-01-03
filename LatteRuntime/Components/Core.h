
#ifndef LATTE_CORE_H
#define LATTE_CORE_H


#include <sol/sol.hpp>

namespace latte
{
	void bindCoreFunctions(sol::state_view state);

	bool loadDependencyScripts(sol::state_view state);
}

#endif // LATTE_CORE_H