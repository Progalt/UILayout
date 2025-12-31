
#ifndef LATTE_FONT_METRICS_H
#define LATTE_FONT_METRICS_H

#include <sol/sol.hpp>
#include <string>

namespace latte
{

	/*
		Interface to return data about a font.

		Useful for determining how to lay stuff out around text
	*/
	class FontMetrics
	{
	public:

		FontMetrics(const std::string& fontName, float fontSize, sol::state_view state);

		/*
			Returns a Lua table with [width] and [height] of the specified string as text
		*/
		sol::table getTextSize(const std::string& str);

		/*
			Registers the usertype with lua
		*/
		static void luaRegister(sol::state_view state);

	private:

		std::string m_FontName = "";
		float m_FontSize;
		sol::state_view m_StateView;
	};
}

#endif // LATTE_FONT_METRICS_H