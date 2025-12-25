#include "EventLoop.h"
#include "../Rendering/NodeRenderer.h"

namespace latte
{
	void EventLoop::runEventLoop()
	{


		bool shouldRun = true;

		SDL_Event evnt{};
		while (SDL_WaitEvent(&evnt) && shouldRun)
		{
			handleEvents(&evnt);

			shouldRun = WindowManager::getInstance().isSomeWindowOpen();


			WindowManager::getInstance().foreach([&](std::shared_ptr<Window> win)
				{
					

					latte::renderRoot(win);

					win->present();
				});
		}



	}

	void EventLoop::handleEvents(SDL_Event* evnt) 
	{
		if (evnt->type >= SDL_EVENT_WINDOW_FIRST && evnt->type <= SDL_EVENT_WINDOW_LAST)
		{
			// Window event
			// Pass to the window so it can handle it
			const uint32_t id = evnt->window.windowID;
			std::shared_ptr<Window> win = WindowManager::getInstance().getWindowById(id);
			if (win && win->valid())
			{
				bool shouldStay = win->handleEvents(evnt);

				// If a close is returned, close the window
				// By removing it from the window manager
				// This will destroy it since its ref counted
				if (!shouldStay)
				{
					WindowManager::getInstance().removeWindow(id);
				}
			}

			return;
		}

		// Handle more global events here
		switch (evnt->type)
		{
			
		}
	}
}