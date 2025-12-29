#define SOL_PRINT_ERRORS 1
#include <sol/sol.hpp>
#include <SDL3/SDL.h>
#include "OS/EventLoop.h"
#include <iostream>
#include "Components/Component.h"
#include "Rendering/NodeRenderer.h"
#include "Utils/Log.h"
#include <nanovg.h>
#include "Router/Router.h"

sol::state state{};

int main(int argc, char* argv)
{
	latte::Log::log(latte::Log::Severity::Info, "LatteUI v0.1");

	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		latte::Log::log(latte::Log::Severity::Fatal, "Failed to initialise SDL3: {}", SDL_GetError());
		return -1;
	}

	latte::engine_event_type_base = SDL_RegisterEvents(latte::ENGINE_EVENT_COUNT);

	state.open_libraries(sol::lib::base, sol::lib::coroutine, sol::lib::string, sol::lib::io, sol::lib::ffi, sol::lib::jit);
	latte::ComponentSystem::getInstance().setState(&state);

	sol::table latteTable = state.create_named_table("latte");

	latte::Router::luaRegister(state);

	latteTable["ui"] = latteTable.create_named("ui");

	latteTable["runApp"] = [&](){
		latte::Log::log(latte::Log::Severity::Info, "Running App Event Loop");

		latte::EventLoop::getInstance().runEventLoop();
	};

	latteTable["useRouter"] = [&](latte::Router& router) {

		const latte::ActiveRoute& route = router.getActiveRoute();
		sol::protected_function builder = router.getRouteFunction(route.pattern);

		if (builder == sol::nil)
		{
			latte::Log::log(latte::Log::Severity::Warning, "Cannot build route if it has no builder func: {}", route.pattern);
			return;
		}



		sol::object obj = builder();
		if (obj.get_type() != sol::type::table)
		{
			latte::Log::log(latte::Log::Severity::Warning, "Route Builder must return a valid table: {}", route.pattern);
			return;
		}

		sol::table table = obj.as<sol::table>();

		std::shared_ptr<latte::Window> win = std::make_shared<latte::Window>(route.pattern, 500, 400,
			latte::WINDOW_FLAG_OPENGL |
			latte::WINDOW_FLAG_RESIZABLE // |
			// backdropFlag
		);
		win->setLuaRootTable(table);
		latte::EventLoop::getInstance().getWindowManager().registerWindow(win);
		router.setWindow(win);

		latte::EventLoop::getInstance().pushRelayout(win);

		
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


		std::shared_ptr<latte::Window> win = std::make_shared<latte::Window>(title, desW, desH,
			latte::WINDOW_FLAG_OPENGL |
			latte::WINDOW_FLAG_RESIZABLE |
			backdropFlag
		);
		win->setLuaRootTable(table);
		latte::EventLoop::getInstance().getWindowManager().registerWindow(win);

		latte::EventLoop::getInstance().pushRelayout(win);

	};

	latteTable["registerComponent"] = [&](const std::string& name, sol::function func, sol::optional<std::string> uiLibName) {
		std::string libName = uiLibName.value_or("ui");
		latte::ComponentSystem::getInstance().registerComponent(name, func, libName);
	};

	latteTable["useState"] = [&](sol::table input_table) {
		std::string str = latte::ComponentSystem::getInstance().getCurrentID();
		LatteNode* node = latte::ComponentSystem::getInstance().findNode(str);
		if (node == nullptr)
			return input_table;

		sol::table& stored_table =
			((latte::ComponentData*)latteGetUserData(node))->state;

		if (!stored_table.valid() || stored_table == sol::nil) {
			stored_table = state.create_table();

			// Merge input_table fields, only on first creation
			if (input_table.valid()) {
				for (const auto& kv : input_table) {
					stored_table.set(kv.first, kv.second);
				}
			}
		}

		stored_table["__latte_node_id"] = str;
		stored_table["__latte_magic"] = "latte_state_table";

		stored_table["setState"] = [](sol::this_state s, sol::table self, sol::table new_state) {
			std::string id = self["__latte_node_id"];
			LatteNode* node = latte::ComponentSystem::getInstance().findNode(id);
			if (node == nullptr)
				return;

			// Always get the latest state table directly from node
			sol::table node_stored_table = ((latte::ComponentData*)latteGetUserData(node))->state;

			for (const auto& kv : new_state) 
			{
				// Convert key to string
				std::string key_str;
				if (kv.first.is<int>()) {
					key_str = std::to_string(kv.first.as<int>());
				}
				else if (kv.first.is<std::string>()) {
					key_str = kv.first.as<std::string>();
				}
				else {
					key_str = "<unknown type>";
				}

				// Convert value to string for debug (primitive types only here)
				std::string value_str;
				if (kv.second.is<bool>()) {
					value_str = kv.second.as<bool>() ? "true" : "false";
				}
				else if (kv.second.is<int>()) {
					value_str = std::to_string(kv.second.as<int>());
				}
				else if (kv.second.is<double>()) {
					value_str = std::to_string(kv.second.as<double>());
				}
				else if (kv.second.is<std::string>()) {
					value_str = kv.second.as<std::string>();
				}
				else if (kv.second.is<sol::table>()) {
					value_str = "<table>";
				}
				else if (kv.second.is<sol::function>()) {
					value_str = "<function>";
				}
				else {
					value_str = "<unknown type>";
				}

				latte::Log::log(latte::Log::Severity::Info, "setState: Setting \"{}\" = {}", key_str, value_str);

				node_stored_table.set(kv.first, kv.second);
			
			}

			latte::EventLoop::getInstance().getWindowManager().foreach([&](std::shared_ptr<latte::Window> win) {
				// win->layout();

				latte::EventLoop::getInstance().pushRelayout(win);
			});

			};

		// Return the persistent state table
		return stored_table;
	};
	 

	latteTable["getID"] = [&]() {

		return latte::ComponentSystem::getInstance().getCurrentID();
	};

	sol::table focus = latteTable.create_named("focus");

	// Register the current component as something that can have focus
	focus["register"] = [&]() -> sol::table {
		std::string str = latte::ComponentSystem::getInstance().getCurrentID();
		LatteNode* node = latte::ComponentSystem::getInstance().findNode(str);

		if (!node)
			return sol::nil;

		sol::table ret = state.create_table();
		ret["id"] = str;

		return ret;
	};

	focus["isFocused"] = [&](sol::object tobj) -> bool {
		if (!tobj.is<sol::table>()) 
		{
			return false;
		}
		// Safe to cast
		sol::table t = tobj.as<sol::table>();
		std::string id = t.get_or("id", std::string());
		if (id.empty())
			return false;
		LatteNode* focusedNode = latte::ComponentSystem::getInstance().getFocusedNode();
		if (focusedNode == nullptr)
			return false;

		return std::string(focusedNode->id) == id;
	};

	focus["request"] = [&](sol::table focusTable) {
		std::string id = focusTable.get_or("id", std::string());

		if (id.empty())
			return;

		LatteNode* node = latte::ComponentSystem::getInstance().findNode(id);

		if (!node)
			return;

		latte::ComponentSystem::getInstance().setFocusedNode(node);
		latte::Log::log(latte::Log::Severity::Info, "Set node has focus: {}", id);
	};

	latteTable["loadComponents"] = [&](const std::string& name) {
	};

	latteTable["measureTextWidth"] = [&](const std::string& str, float fontSize){

		NVGcontext* vg = latte::RenderInterface::getInstance().getNVGContext();
		
		nvgFontFace(vg, "Roboto-Regular");
		nvgFontSize(vg, fontSize);

		float bb[4];
		return nvgTextBounds(vg, 0.0f, 0.0f, str.c_str(), NULL, bb);
	};

	{
		auto result = state.do_file("luaSrc/latte-base.lua");

		if (!result.valid()) {
			sol::error err = result;
			latte::Log::log(latte::Log::Severity::Error, "{}", std::string(err.what()));
		}
	}

	{
		auto result = state.do_file("luaSrc/latte-components.lua");

		if (!result.valid()) {
			sol::error err = result;
			latte::Log::log(latte::Log::Severity::Error, "{}", std::string(err.what()));
		}
	}

	{
		auto result = state.do_file("luaSrc/latte-fluentui.lua");

		if (!result.valid()) {
			sol::error err = result;
			latte::Log::log(latte::Log::Severity::Error, "{}", std::string(err.what()));
		}
	}

	{
		auto result = state.do_file("Tests/CounterApp.lua");
		// auto result = state.do_file("Tests/SimpleWindow.lua");

		if (!result.valid()) {
			sol::error err = result;
			latte::Log::log(latte::Log::Severity::Error, "{}", std::string(err.what()));
		}
	}
	


	SDL_Quit();
	return 0;
}