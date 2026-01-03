#include "latteui.h"
#include <sol/sol.hpp>
#include <SDL3/SDL.h>
#include "OS/EventLoop.h"
#include "Components/Component.h"
#include "Utils/Log.h"
#include "Router/Router.h"
#include "Components/Focus.h"
#include "Rendering/FontMetrics.h"
#include "OS/Clipboard.h"
#include "Components/Core.h"

namespace latte
{
	sol::state state{};
	std::string g_ScriptPath = "";

	void run_lua_file_with_logging(sol::state& state, const std::string& path) {
		sol::load_result loaded = state.load_file(path);
		if (!loaded.valid()) {
			sol::error err = loaded;
			latte::Log::log(latte::Log::Severity::Error, "Load error in '{}': {}", path, err.what());
			return;
		}

		sol::protected_function func = loaded;
		sol::protected_function_result result = func();

		if (!result.valid()) {
			sol::error err = result;
			latte::Log::log(latte::Log::Severity::Error, "Runtime error in '{}': {}", path, err.what());
			return;
		}
	}

	bool runScript(const std::string& path)
	{
		g_ScriptPath = path;

		latte::Log::log(latte::Log::Severity::Info, "LatteUI v{}.{}", LATTEUI_MAJOR_VERSION, LATTEUI_MINOR_VERSION);

		if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
		{
			latte::Log::log(latte::Log::Severity::Fatal, "Failed to initialise SDL3: {}", SDL_GetError());
			return false;
		}

		latte::engine_event_type_base = SDL_RegisterEvents(latte::ENGINE_EVENT_COUNT);

		state.open_libraries(
			sol::lib::base,
			sol::lib::os,
			sol::lib::math,
			sol::lib::table,
			sol::lib::package,
			sol::lib::coroutine,
			sol::lib::string,
			sol::lib::io,
			sol::lib::ffi,
			sol::lib::jit
		);

		latte::ComponentSystem::getInstance().setState(&state);

		latte::bindCoreFunctions(state);

		latte::Router::luaRegister(state);
		latte::ComponentLibrary::luaRegister(state);
		latte::Focus::luaRegister(state);
		latte::FontMetrics::luaRegister(state);
		latte::Clipboard::luaRegister(state);

		latte::loadDependencyScripts(state);

		run_lua_file_with_logging(state, path);

		latte::Log::log(latte::Log::Severity::Info, "Running App Event Loop");
		latte::EventLoop::getInstance().runEventLoop(state);


		SDL_Quit();
		return true;
	}

	bool hotRestart()
	{
		if (g_ScriptPath.empty())
			return false;

		state = sol::state{};

	}
}