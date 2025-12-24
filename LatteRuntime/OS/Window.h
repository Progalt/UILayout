
#ifndef LATTE_WINDOW_H
#define LATTE_WINDOW_H

#include <SDL3/SDL.h>
extern "C" {
#include <LatteLayout/layout.h>
}
#include <string>
#include <sol/sol.hpp>

namespace latte
{
	enum WindowFlags
	{
		WINDOW_FLAG_NONE = 0,
		WINDOW_FLAG_OPENGL = 1 , 
		WINDOW_FLAG_RESIZABLE = 2
	};

	class Window
	{
	public:

		Window(const std::string& title, int w, int h, int flags);
		~Window();

		void handleEvents(SDL_Event* evnt);

		void makeCurrent();

		void present();

		[[nodiscard]] const uint32_t getWindowID() const noexcept { return m_Id; }

		[[nodiscard]] const bool isOpen() const noexcept { return m_IsOpen; }

		[[nodiscard]] LatteNode* getRootNode() const noexcept { return m_RootNode; }

		[[nodiscard]] const int getWidth() const noexcept { return m_Width; }
		[[nodiscard]] const int getHeight() const noexcept { return m_Height; }

		void setLuaRootTable(sol::table table) noexcept { m_RootTable = table; }
		[[nodiscard]] sol::table getLuaRootTable() const noexcept { return m_RootTable; }

		void layout();

	private:

		SDL_Window* m_Window = nullptr;
		SDL_GLContext m_Context = nullptr;

		uint32_t m_Id = 0;
		bool m_IsOpen = false;

		int m_Width = 0;
		int m_Height = 0;

		LatteNode* m_RootNode = nullptr;
		sol::table m_RootTable = {};
	};
}


#endif // LATTE_WINDOW_H