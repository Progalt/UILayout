
#ifndef LATTE_EVENTLOOP_H
#define LATTE_EVENTLOOP_H

#include "../Utils/Singleton.h"
#include "WindowManager.h"
#include <SDL3/SDL.h>

namespace latte
{
	class EventLoop : public Singleton<EventLoop>
	{
	public:

		void runEventLoop();

	private:

		void handleEvents(SDL_Event* evnt);
	};
}

#endif // LATTE_EVENTLOOP_H