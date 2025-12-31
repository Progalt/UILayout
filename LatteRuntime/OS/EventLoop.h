
#ifndef LATTE_EVENTLOOP_H
#define LATTE_EVENTLOOP_H

#include "../Utils/Singleton.h"
#include "WindowManager.h"
#include <SDL3/SDL.h>
#include <variant>
#include "Event.h"

namespace latte
{

	enum EngineEvents {
		ENGINE_EVENT_RELAYOUT = 0,
		ENGINE_EVENT_REPAINT = 1,
		ENGINE_EVENT_COUNT
	};

	extern Uint32 engine_event_type_base;

	class EventLoop : public Singleton<EventLoop>
	{
	public:

		void runEventLoop(sol::state_view state);

		[[nodiscard]] WindowManager& getWindowManager() noexcept { return m_WindowManager; }

		void pushRepaint(std::shared_ptr<Window> win);
		void pushRelayout(std::shared_ptr<Window> win);

	private:

		void handleEvents(SDL_Event* evnt, sol::state_view state);
		
		WindowManager m_WindowManager;
	};
}

#endif // LATTE_EVENTLOOP_H