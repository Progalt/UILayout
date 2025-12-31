
#include "Clipboard.h"
#include <SDL3/SDL.h>

namespace latte
{
	void Clipboard::setText(const std::string& str) noexcept
	{
		SDL_SetClipboardText(str.c_str());
	}

	[[nodiscard]] std::string Clipboard::getText() noexcept
	{
		if (!SDL_HasClipboardText())
			return std::string();

		char* str = SDL_GetClipboardText();

		std::string ret(str);
		SDL_free(str);

		return ret;
	}

	void Clipboard::luaRegister(sol::state_view state)
	{
		state.new_usertype<Clipboard>("Clipboard",
			"setText", &Clipboard::setText,
			"getText", &Clipboard::getText
		);
	}
}