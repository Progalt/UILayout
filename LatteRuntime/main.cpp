
#include <sol/sol.hpp>
#include <SDL3/SDL.h>
#include "OS/EventLoop.h"
#include <iostream>
#include "Binding/Component.h"

sol::state state{};

int main(int argc, char* argv)
{

	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		return -1;
	}

	state.open_libraries(sol::lib::base);
	latte::ComponentSystem::getInstance().setState(&state);

	sol::table latteTable = state.create_named_table("latte");

	latteTable["ui"] = latteTable.create_named("ui");

	latteTable["runApp"] = [&](){
		printf("Running Event Loop\n");
		latte::EventLoop::getInstance().runEventLoop();
	};

	latteTable["showWindow"] = [&](sol::table table) {
		const std::string title = table.get_or<std::string>("title", "LatteUI Window");
		printf("Creating window with title %s\n", title.c_str());

		int desW = 300, desH = 200;

		if (table["size"].valid()) 
		{
			sol::table sizeTable = table["size"];
			if (sizeTable[1].valid()) desW = sizeTable[1];
			if (sizeTable[2].valid()) desH = sizeTable[2];
		}


		std::shared_ptr<latte::Window> win = std::make_shared<latte::Window>(title, desW, desH, latte::WINDOW_FLAG_OPENGL | latte::WINDOW_FLAG_RESIZABLE);
		win->setLuaRootTable(table);

		latte::ComponentSystem::getInstance().pushID(win->getRootNode()->id);

		latte::applyPropsFromTable(win->getRootNode(), table);

		latte::ComponentSystem::getInstance().popID();

		latteLayout(win->getRootNode());

		latte::WindowManager::getInstance().registerWindow(win);
	};

	latteTable["registerComponent"] = [&](const std::string& name, sol::function func) {

		latte::ComponentSystem::getInstance().registerComponent(name, func);

	};

	latteTable["useState"] = [&](sol::table table) {

		return table;
	};

	latteTable["getID"] = [&]() {

		return latte::ComponentSystem::getInstance().getCurrentID();
	};


	auto result = state.do_file("Tests/SimpleWindow.lua");

	if (!result.valid()) {
		sol::error err = result;
		std::cout << "Error: " << err.what() << std::endl;
	}
	


	SDL_Quit();
	return 0;
}