#ifndef LATTE_CLIPBOARD_H
#define LATTE_CLIPBOARD_H

#include <sol/sol.hpp>

namespace latte
{
	class Clipboard
	{
	public:

		static void setText(const std::string& str) noexcept;
		[[nodiscard]] static std::string getText() noexcept;

		static void luaRegister(sol::state_view state);
	};
}

#endif // LATTE_CLIPBOARD_H