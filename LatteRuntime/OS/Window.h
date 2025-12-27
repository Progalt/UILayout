
#ifndef LATTE_WINDOW_H
#define LATTE_WINDOW_H

#include <SDL3/SDL.h>
extern "C" {
#include <LatteLayout/layout.h>
}
#include <string>
#include <sol/sol.hpp>

#ifdef _WIN32
#include <Windows.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#endif

namespace latte
{
	enum WindowFlags : uint64_t
	{
		WINDOW_FLAG_NONE = 0,
		WINDOW_FLAG_OPENGL = 1 << 0, 
		WINDOW_FLAG_RESIZABLE = 1 << 1,


		WINDOW_FLAG_NOTITLEBAR = 1 << 2,
		WINDOW_FLAG_TRANSPARENT = 1 << 3,

		// Windows 11 Only
		// Converts the background of the window to full Mica
		WINDOW_FLAG_MICA = 1 << 16
	};

	class Window
	{
	public:

		Window(const std::string& title, int w, int h, int flags);
		~Window();

		bool handleEvents(SDL_Event* evnt);

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

		[[nodiscard]] bool valid() const noexcept
		{
			return m_Window != nullptr;
		}

#ifdef _WIN32
		HWND getPlatformWindowHandle();
#endif

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