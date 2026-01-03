// #define SOL_PRINT_ERRORS 1
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

sol::state state{};

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

int main(int argc, char* argv)
{
	latte::Log::log(latte::Log::Severity::Info, "LatteUI v0.1");

	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
	{
		latte::Log::log(latte::Log::Severity::Fatal, "Failed to initialise SDL3: {}", SDL_GetError());
		return -1;
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

	latte::loadDependencyScripts(state, "luaSrc");

	run_lua_file_with_logging(state, "Tests/CounterApp.lua");
	
	latte::Log::log(latte::Log::Severity::Info, "Running App Event Loop");
	latte::EventLoop::getInstance().runEventLoop(state);


	SDL_Quit();
	return 0;
}