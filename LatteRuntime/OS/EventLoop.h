
#ifndef LATTE_EVENTLOOP_H
#define LATTE_EVENTLOOP_H

#include "../Utils/Singleton.h"
#include "WindowManager.h"
#include <SDL3/SDL.h>
#include <variant>
#include "Event.h"

namespace latte
{
	

	class EventLoop : public Singleton<EventLoop>
	{
	public:

		void runEventLoop();

		[[nodiscard]] WindowManager& getWindowManager() noexcept { return m_WindowManager; }

	private:

		void handleEvents(SDL_Event* evnt);
		
		WindowManager m_WindowManager;
	};
}

#endif // LATTE_EVENTLOOP_H