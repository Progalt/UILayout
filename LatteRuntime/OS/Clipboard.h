#ifndef LATTE_CLIPBOARD_H
#define LATTE_CLIPBOARD_H

#include <sol/sol.hpp>

namespace latte
{
	/*
		Clipboard interface
	*/
	class Clipboard
	{
	public:

		/*
			Sets the text in the clipboard
		*/
		static void setText(const std::string& str) noexcept;

		/*
			Returns a string from the clipboard, 
			if nothing is in the clipboard this returns an empty string
		*/
		[[nodiscard]] static std::string getText() noexcept;

		static void luaRegister(sol::state_view state);
	};
}

#endif // LATTE_CLIPBOARD_H