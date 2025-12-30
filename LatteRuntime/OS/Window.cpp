#include "Window.h"
#include <stdexcept>
#include "../Components/Component.h"
#include "../Rendering/NodeRenderer.h"


namespace latte
{
	static SDL_GLContext sharedContext = nullptr;

	Window::Window(const std::string& title, int w, int h, int flags)
	{

		int f = 0;

		if (flags & WINDOW_FLAG_OPENGL)
		{
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
			SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
			SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

			// Only set sharing if there's already a context available
			if (sharedContext != nullptr)
			{
				SDL_GL_MakeCurrent(nullptr, sharedContext); // Make the shared context current
			}

			f |= SDL_WINDOW_OPENGL;
		}

		if (flags & WINDOW_FLAG_RESIZABLE)
			f |= SDL_WINDOW_RESIZABLE;

		if (flags & WINDOW_FLAG_TRANSPARENT)
			f |= SDL_WINDOW_TRANSPARENT;

		if (flags & WINDOW_FLAG_NOTITLEBAR)
			f |= SDL_WINDOW_BORDERLESS;

		m_Window = SDL_CreateWindow(title.c_str(), w, h, f);

		if (!m_Window)
		{
			throw std::runtime_error("Failed to create SDL3 Window");
		}

		if (flags & WINDOW_FLAG_OPENGL)
		{
			m_Context = SDL_GL_CreateContext(m_Window);

			if (!m_Context)
			{
				throw std::runtime_error("Failed to create OpenGL context: " +
					std::string(SDL_GetError()));
			}

			// Store the first context as the shared context
			if (sharedContext == nullptr)
			{
				sharedContext = m_Context;

				SDL_GL_MakeCurrent(m_Window, m_Context);
				latte::RenderInterface::getInstance().getNVGContext();
			}
		}

		m_RootNode = latteCreateNode((title + "_root").c_str(), nullptr, LATTE_NODE_FLAGS_DELETE_USERDATA);

		m_IsOpen = true;

		m_Id = SDL_GetWindowID(m_Window);

		SDL_GetWindowSizeInPixels(m_Window, &m_Width, &m_Height);
		SDL_StartTextInput(m_Window);

#ifdef _WIN32

		HWND hwnd = getPlatformWindowHandle();

		bool extendClientArea = false;

		if (flags & WINDOW_FLAG_NOTITLEBAR)
		{

			LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);

			// Re-enable styles that allow snapping (WS_CAPTION, WS_THICKFRAME)
			style |= (WS_CAPTION | WS_THICKFRAME);

			// Apply new style
			SetWindowLongPtr(hwnd, GWL_STYLE, style);
			SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
				SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

			extendClientArea = true;
		}

		if (flags & WINDOW_FLAG_MICA)
		{
			DWM_SYSTEMBACKDROP_TYPE backdropType = DWMSBT_MAINWINDOW;

			HRESULT hr = DwmSetWindowAttribute(
				hwnd,
				DWMWA_SYSTEMBACKDROP_TYPE,
				&backdropType,
				sizeof(backdropType)
			);

			extendClientArea = true;
		}

		// TODO: Acrylic backdrops
		// Also figure out how to get this more customisable on Win32

		if (extendClientArea)
		{
			MARGINS margins = { -1, -1, -1, -1 };
			DwmExtendFrameIntoClientArea(hwnd, &margins);
		}

		// Acrylic
		//DWM_SYSTEMBACKDROP_TYPE backdropType = DWMSBT_TRANSIENTWINDOW;

#endif
	}

	Window::~Window()
	{
		if (m_Window)
		{
			if (m_Context)
			{
				SDL_GL_DestroyContext(m_Context);
			}

			SDL_DestroyWindow(m_Window);
		}

		latteFreeNode(m_RootNode);
	}

	void Window::present()
	{
		SDL_GL_SwapWindow(m_Window);
	}

	void Window::makeCurrent()
	{
		SDL_GL_MakeCurrent(m_Window, m_Context);
	}

	void Window::layout()
	{
		latteSizer(
			m_RootNode,
			LATTE_SIZER_FIXED((float)m_Width),
			LATTE_SIZER_FIXED((float)m_Height)
		);

		latte::ComponentSystem::getInstance().pushID(m_RootNode->id);

		if (m_RootTable != sol::nil)
		{
			latte::applyPropsFromTable(m_RootNode, m_RootTable, false);
		}

		latte::ComponentSystem::getInstance().popID();

		lattePropogateDirty(m_RootNode);

		latteLayout(m_RootNode);
	}

	bool Window::handleEvents(SDL_Event* evnt)
	{

		switch (evnt->type)
		{
		case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
		{
			m_IsOpen = false;
			return false;
			break;
		}
		case SDL_EVENT_WINDOW_RESIZED:
		{
			m_Width = evnt->window.data1;
			m_Height = evnt->window.data2;
			break;
		}
		}

		return true;
	}

#ifdef _WIN32
	HWND Window::getPlatformWindowHandle()
	{
		HWND hwnd = (HWND)SDL_GetPointerProperty(
			SDL_GetWindowProperties(m_Window),
			SDL_PROP_WINDOW_WIN32_HWND_POINTER,
			NULL
		);

		return hwnd;
	}
#endif
}