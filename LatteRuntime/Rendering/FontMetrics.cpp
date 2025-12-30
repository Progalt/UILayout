#include "FontMetrics.h"
#include "NodeRenderer.h"
#include <nanovg.h>

namespace latte
{
	FontMetrics::FontMetrics(const std::string& fontName, float fontSize, sol::state_view state)
		: m_FontName(fontName), m_FontSize(fontSize), m_StateView(state)
	{
	}

	sol::table FontMetrics::getTextSize(const std::string& str)
	{
		NVGcontext* vg = RenderInterface::getInstance().getNVGContext();

		nvgFontFace(vg, m_FontName.c_str());
		nvgFontSize(vg, m_FontSize);

		float bb[4];
		float w = nvgTextBounds(vg, 0.0f, 0.0f, str.c_str(), nullptr, bb);
		float h = bb[3] - bb[1];

		sol::table result = m_StateView.create_table();

		result["width"] = w;
		result["height"] = h;

		return result;
	}

	void FontMetrics::luaRegister(sol::state_view state)
	{
		state.new_usertype<latte::FontMetrics>(
			"FontMetrics",
			"getTextSize", &latte::FontMetrics::getTextSize
		);
	}
}