
#ifndef LATTE_FONT_METRICS_H
#define LATTE_FONT_METRICS_H

#include <sol/sol.hpp>
#include <string>

namespace latte
{
	class FontMetrics
	{
	public:

		FontMetrics(const std::string& fontName, float fontSize, sol::state_view state);

		sol::table getTextSize(const std::string& str);

		static void luaRegister(sol::state_view state);

	private:

		std::string m_FontName = "";
		float m_FontSize;
		sol::state_view m_StateView;
	};
}

#endif // LATTE_FONT_METRICS_H