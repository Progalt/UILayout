#include "Window.h"
#include <stdexcept>
#include "../Binding/Component.h"

namespace latte
{
	Window::Window(const std::string& title, int w, int h, int flags)
	{

		int f = 0;

		if (flags & WINDOW_FLAG_OPENGL)
		{
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
			SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);

			f |= SDL_WINDOW_OPENGL;
		}

		if (flags & WINDOW_FLAG_RESIZABLE)
			f |= SDL_WINDOW_RESIZABLE;

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

			}
		}

		m_RootNode = latteCreateNode((title + "_root").c_str(), nullptr, LATTE_NODE_FLAGS_DELETE_USERDATA);

		latteSizer(
			m_RootNode,
			LATTE_SIZER_FIXED((float)m_Width),
			LATTE_SIZER_FIXED((float)m_Height)
		);

		m_IsOpen = true;

		m_Id = SDL_GetWindowID(m_Window);

		SDL_GetWindowSizeInPixels(m_Window, &m_Width, &m_Height);
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

	void Window::handleEvents(SDL_Event* evnt)
	{

		switch (evnt->type)
		{
		case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
			m_IsOpen = false;
			SDL_ShowWindow(m_Window);
			break;
		case SDL_EVENT_WINDOW_RESIZED:
		{
			m_Width = evnt->window.data1;
			m_Height = evnt->window.data2;


			latteSizer(
				m_RootNode,
				LATTE_SIZER_FIXED((float)m_Width),
				LATTE_SIZER_FIXED((float)m_Height)
			);

			latte::ComponentSystem::getInstance().pushID(m_RootNode->id);

			latte::applyPropsFromTable(m_RootNode, m_RootTable, false);

			latte::ComponentSystem::getInstance().popID();
			latteLayout(m_RootNode);

			break;
		}
		}

	}
}