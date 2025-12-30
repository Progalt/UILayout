
#ifndef LATTE_FOCUS_H
#define LATTE_FOCUS_H

#include <sol/sol.hpp>

namespace latte
{
	class Focus
	{
	public:

		Focus(const std::string& id);

		bool isFocused();

		void request();

		static void luaRegister(sol::state_view state);

	private:

		std::string m_ID = "";
	};
}

#endif // LATTE_FOCUS_H