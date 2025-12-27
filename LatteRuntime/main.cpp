
#include <sol/sol.hpp>
#include <SDL3/SDL.h>
#include "OS/EventLoop.h"
#include <iostream>
#include "Binding/Component.h"
#include "Rendering/NodeRenderer.h"
#include "Utils/Log.h"

sol::state state{};

int main(int argc, char* argv)
{
	latte::Log::log(latte::Log::Severity::Info, "LatteUI v0.1");

	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		latte::Log::log(latte::Log::Severity::Fatal, "Failed to initialise SDL3: {}", SDL_GetError());
		return -1;
	}

	state.open_libraries(sol::lib::base, sol::lib::coroutine, sol::lib::string, sol::lib::io, sol::lib::ffi, sol::lib::jit);
	latte::ComponentSystem::getInstance().setState(&state);

	sol::table latteTable = state.create_named_table("latte");

	latteTable["ui"] = latteTable.create_named("ui");

	latteTable["runApp"] = [&](){
		latte::Log::log(latte::Log::Severity::Info, "Running App Event Loop");

		latte::EventLoop::getInstance().runEventLoop();
	};

	latteTable["showWindow"] = [&](sol::table table) {
		const std::string title = table.get_or<std::string>("title", "LatteUI Window");
		latte::Log::log(latte::Log::Severity::Info, "Creating window: {}", title);

		int desW = 300, desH = 200;

		if (table["size"].valid()) 
		{
			sol::table sizeTable = table["size"];
			if (sizeTable[1].valid()) desW = sizeTable[1];
			if (sizeTable[2].valid()) desH = sizeTable[2];
		}

		sol::object backdrop = table["backdrop"];

		int backdropFlag = 0;

		if (backdrop.valid())
		{
			auto assignBackdropFlagFromLua = [&](int luaFlag)
				{
					switch (luaFlag)
					{
					case 0:
						break;
					case 1:	// Mica
						backdropFlag = latte::WINDOW_FLAG_MICA;
						break;
					case 2:	// Acrylic (TODO)
						break;
					case 3:	// Transparent
						backdropFlag = latte::WINDOW_FLAG_TRANSPARENT;
						break;
					}
				};

			if (backdrop.get_type() == sol::type::table)
			{
				sol::table bd = backdrop.as<sol::table>();
				int flag = bd.get_or("type", 0);

				assignBackdropFlagFromLua(flag);
			}
			else if (backdrop.get_type() == sol::type::number)
			{
				int flag = backdrop.as<int>();

				assignBackdropFlagFromLua(flag);
			}
		}


		std::shared_ptr<latte::Window> win = std::make_shared<latte::Window>(title, desW, desH, latte::WINDOW_FLAG_OPENGL | latte::WINDOW_FLAG_RESIZABLE | backdropFlag);
		win->setLuaRootTable(table);

		win->layout();
		latte::EventLoop::getInstance().getWindowManager().registerWindow(win);

	};

	latteTable["registerComponent"] = [&](const std::string& name, sol::function func, sol::optional<std::string> uiLibName) {
		std::string libName = uiLibName.value_or("");
		latte::ComponentSystem::getInstance().registerComponent(name, func, libName);
	};

	latteTable["useState"] = [&](sol::table table) {

		return table;
	};

	latteTable["getID"] = [&]() {

		return latte::ComponentSystem::getInstance().getCurrentID();
	};

	latteTable["loadComponents"] = [&](const std::string& name) {
	};

	{
		auto result = state.do_file("luaSrc/latte-base.lua");

		if (!result.valid()) {
			sol::error err = result;
			latte::Log::log(latte::Log::Severity::Error, std::string(err.what()));
		}
	}

	{
		auto result = state.do_file("luaSrc/latte-components.lua");

		if (!result.valid()) {
			sol::error err = result;
			latte::Log::log(latte::Log::Severity::Error, std::string(err.what()));
		}
	}

	{
		auto result = state.do_file("Tests/SimpleWindow.lua");

		if (!result.valid()) {
			sol::error err = result;
			latte::Log::log(latte::Log::Severity::Error, std::string(err.what()));
		}
	}
	


	SDL_Quit();
	return 0;
}