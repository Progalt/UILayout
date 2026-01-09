#include "EventLoop.h"
#include "../Rendering/NodeRenderer.h"
#include "../Components/ComponentEvents.h"
#include "../Components/Component.h"

namespace latte
{
	Uint32 engine_event_type_base;

	std::vector<std::string> getKeyModNames(SDL_Keymod mod) 
	{
		std::vector<std::string> mods;
		if (mod & SDL_KMOD_LCTRL)  mods.push_back("left ctrl");
		if (mod & SDL_KMOD_RCTRL)  mods.push_back("right ctrl");
		if (mod & SDL_KMOD_LSHIFT) mods.push_back("left shift");
		if (mod & SDL_KMOD_RSHIFT) mods.push_back("right shift");
		if (mod & SDL_KMOD_LALT)   mods.push_back("left alt");
		if (mod & SDL_KMOD_RALT)   mods.push_back("right alt");
		if (mod & SDL_KMOD_LGUI)   mods.push_back("left gui");
		if (mod & SDL_KMOD_RGUI)   mods.push_back("right gui");
		/*if (mod & SDL_KMOD_NUM)    mods.push_back("num");
		if (mod & SDL_KMOD_CAPS)   mods.push_back("caps");
		if (mod & SDL_KMOD_MODE)   mods.push_back("mode");*/
		return mods;
	}

	void EventLoop::runEventLoop(sol::state_view state)
	{
		bool shouldRun = true;

		SDL_Event evnt{};
		while (SDL_WaitEvent(&evnt) && shouldRun)
		{
			handleEvents(&evnt, state);

			shouldRun = m_WindowManager.isSomeWindowOpen();

			if (!shouldRun)
				Log::log(Log::Severity::Info, "No Windows left open");
		}

		Log::log(Log::Severity::Info, "Quitting event loop");

	}

	void EventLoop::pushRepaint(std::shared_ptr<Window> win)
	{
		auto* window_ptr = new std::shared_ptr<Window>(win);

		SDL_Event evnt{};
		evnt.type = engine_event_type_base + ENGINE_EVENT_REPAINT;
		evnt.user.data1 = window_ptr;
		SDL_PushEvent(&evnt);
	}

	void EventLoop::pushRelayout(std::shared_ptr<Window> win)
	{
		auto* window_ptr = new std::shared_ptr<Window>(win);

		SDL_Event evnt{};
		evnt.type = engine_event_type_base + ENGINE_EVENT_RELAYOUT;
		evnt.user.data1 = window_ptr;
		SDL_PushEvent(&evnt);
	}

	void EventLoop::handleEvents(SDL_Event* evnt, sol::state_view state)
	{
		if (evnt->type >= SDL_EVENT_WINDOW_FIRST && evnt->type <= SDL_EVENT_WINDOW_LAST)
		{
			// Window event
			// Pass to the window so it can handle it
			const uint32_t id = evnt->window.windowID;
			std::shared_ptr<Window> win = m_WindowManager.getWindowById(id);
			if (win && win->valid())
			{
				bool shouldStay = win->handleEvents(evnt);

				if (evnt->type == SDL_EVENT_WINDOW_RESIZED)
				{
					pushRelayout(win);
				}

				// If a close is returned, close the window
				// By removing it from the window manager
				// This will destroy it since its ref counted
				if (!shouldStay)
				{
					m_WindowManager.removeWindow(id);
				}
			}

			return;
		}

		// Handle more global events here
		switch (evnt->type)
		{
		case SDL_EVENT_MOUSE_MOTION:
		{
			MouseMotionEvent mme{};
			mme.x = evnt->motion.x;
			mme.y = evnt->motion.y;
			mme.dx = evnt->motion.xrel;
			mme.dy = evnt->motion.yrel;
			Event latteEvent = mme;

			std::shared_ptr<Window> win = m_WindowManager.getWindowById(evnt->motion.windowID);
			if (win)
				handleNodeEvent(latteEvent, win->getRootNode(), state);
			break;
		}
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
		{
			MouseButtonEvent mbe{};
			mbe.button = (MouseButton)evnt->button.button;
			mbe.state = ButtonState::Down;
			mbe.x = evnt->button.x;
			mbe.y = evnt->button.y;

			Event latteEvent = mbe;
			std::shared_ptr<Window> win = m_WindowManager.getWindowById(evnt->motion.windowID);
			if (win)
				handleNodeEvent(latteEvent, win->getRootNode(), state);
			break;
		}
		case SDL_EVENT_MOUSE_BUTTON_UP:
		{
			MouseButtonEvent mbe{};
			mbe.button = (MouseButton)evnt->button.button;
			mbe.state = ButtonState::Up;
			mbe.x = evnt->button.x;
			mbe.y = evnt->button.y;

			Event latteEvent = mbe;
			std::shared_ptr<Window> win = m_WindowManager.getWindowById(evnt->motion.windowID);
			if (win)
				handleNodeEvent(latteEvent, win->getRootNode(), state);
			break;
		}
		case SDL_EVENT_MOUSE_WHEEL:
			break;
		case SDL_EVENT_KEY_DOWN:
		{
			KeyDownEvent kde{};
			kde.keyCode = evnt->key.key;
			kde.name = std::string(SDL_GetKeyName(kde.keyCode));
			kde.keyMods = getKeyModNames(evnt->key.mod);

			std::transform(
				kde.name.begin(), kde.name.end(), kde.name.begin(),
				[](unsigned char c) { return std::tolower(c); }
			);


			Event latteEvent = kde;
			LatteNode* node = ComponentSystem::getInstance().getFocusedNode();
			if (node)
			{
				handleNodeEvent(latteEvent, node, state);
			}

			break;
		}
		case SDL_EVENT_TEXT_INPUT:
		{
			TextInputEvent tie{};
			tie.str = std::string(evnt->text.text);

			// TODO: Handle window 

			Event latteEvent = tie;
			LatteNode* node = ComponentSystem::getInstance().getFocusedNode();
			if (node)
			{
				handleNodeEvent(latteEvent, node, state);
			}

			break;
		}
		}

		if (evnt->type == engine_event_type_base + ENGINE_EVENT_REPAINT)
		{
			auto* win_sp = (std::shared_ptr<Window>*)evnt->user.data1;
			latte::renderRoot(*win_sp);
			(*win_sp)->present();
			delete win_sp;
		}
		else if (evnt->type == engine_event_type_base + ENGINE_EVENT_RELAYOUT)
		{
			auto* win_sp = (std::shared_ptr<Window>*)evnt->user.data1;
			(*win_sp)->layout();
			pushRepaint(*win_sp); 
			delete win_sp;
		}
	}
}