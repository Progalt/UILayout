#include "Core.h"

#include "Component.h"
#include "../Utils/Log.h"
#include "../Router/Router.h"
#include "../Utils/LuaHelpers.h"
#include "Focus.h"
#include "../Rendering/FontMetrics.h"
#include "../OS/EventLoop.h"

namespace latte
{
	void bindCoreFunctions(sol::state_view state)
	{
		sol::table latteTable = state.create_named_table("latte");

		latteTable["useRouter"] =
			[&](latte::Router& router) -> void {

			// Check if the router is already paired with a window
			if (router.getWindow() != nullptr)
			{
				latte::Log::log(latte::Log::Severity::Warning, "Router already has a window, cannot assign another to it");
				return;
			}

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

			int backdropFlag = 0, desW = 500, desH = 400;
			std::string title = "";

			sol::table winTable = router.getWindowTable();

			if (winTable.get_type() == sol::type::table)
			{
				title = winTable.get_or("title", std::string());

				if (winTable["size"].valid())
				{
					sol::table sizeTable = winTable["size"];
					if (sizeTable[1].valid()) desW = sizeTable[1];
					if (sizeTable[2].valid()) desH = sizeTable[2];
				}

				sol::object backdrop = winTable["backdrop"];

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
			}

			std::shared_ptr<latte::Window> win = std::make_shared<latte::Window>(
				title.empty() ? route.pattern : title,
				desW,
				desH,
				latte::WINDOW_FLAG_OPENGL |
				latte::WINDOW_FLAG_RESIZABLE |
				backdropFlag
			);
			win->setLuaRootTable(table);
			latte::EventLoop::getInstance().getWindowManager().registerWindow(win);
			router.setWindow(win);

			latte::EventLoop::getInstance().pushRelayout(win);


			};

		latteTable["createComponentLibrary"] =
			[](const std::string& name) -> std::shared_ptr<latte::ComponentLibrary> {
			return latte::ComponentSystem::getInstance().createComponentLibrary(name);
			};

		latteTable["useEffect"] =
			[&](sol::protected_function func, sol::table deps) -> void {

			std::string str = latte::ComponentSystem::getInstance().getCurrentID();
			LatteNode* node = latte::ComponentSystem::getInstance().findNode(str);
			if (node == nullptr)
				return;

			latte::ComponentData* data = (latte::ComponentData*)latteGetUserData(node);

			int effectOffset = data->effectOffset;
			if (effectOffset >= data->effects.size())
				data->effects.resize(effectOffset + 1);

			auto& eff = data->effects[effectOffset];

			bool first_render = (eff.dependencies == sol::nil);

			if (first_render)
			{
				eff.func = func;
				eff.dependencies = deps;
				// Call immediately on mount
				eff.func();
			}
			else
			{
				// Check if deps differ
				if (!latte::shallowTableEqual(eff.dependencies, deps))
				{
					eff.func();
					eff.dependencies = deps; // Save updated deps
				}
				// If they are the same, do nothing
			}

			data->effectOffset++;
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

				// Very scuffed needs a fix
				latte::EventLoop::getInstance().getWindowManager().foreach([&](std::shared_ptr<latte::Window> win) {
					latte::EventLoop::getInstance().pushRelayout(win);
					});

				};

			// Return the persistent state table
			return stored_table;
			};


		latteTable["getID"] =
			[&]() -> const std::string& {

			return latte::ComponentSystem::getInstance().getCurrentID();
			};

		latteTable["useFocus"] =
			[&]() -> latte::Focus {

			std::string str = latte::ComponentSystem::getInstance().getCurrentID();
			return latte::Focus(str);
			};

		latteTable["getFontMetrics"] =
			[&](const std::string& fontFace, float size) -> latte::FontMetrics {

			return latte::FontMetrics(fontFace, size, state);
			};
	}

	bool loadDependencyScripts(sol::state_view state, const std::string& basePath)
	{
		// This is ordered, they will get loaded in this order
		// For instance components depends on base and textedit
		std::vector<std::string> deps = {
			"latte-base.lua",
			"latte-textedit.lua",
			"latte-components.lua",
			"latte-fluentui.lua",
			"latte-material3.lua"
		};

		for (std::string& script : deps)
		{
			std::string path = basePath + script;

			sol::load_result loaded = state.load_file(path);
			if (!loaded.valid()) 
			{
				sol::error err = loaded;
				latte::Log::log(latte::Log::Severity::Error, "Load error in '{}': {}", path, err.what());
				return false;
			}

			sol::protected_function func = loaded;
			sol::protected_function_result result = func();

			if (!result.valid()) 
			{
				sol::error err = result;
				latte::Log::log(latte::Log::Severity::Error, "Runtime error in '{}': {}", path, err.what());
				return false;
			}
		}

		return true; 
	}
}