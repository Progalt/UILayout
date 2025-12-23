#include "WindowManager.h"

namespace latte
{
	void WindowManager::registerWindow(std::shared_ptr<Window> window)
	{

		m_Windows[window->getWindowID()] = window;

	}

	bool WindowManager::isSomeWindowOpen()
	{
		for (auto& [id, window] : m_Windows)
		{
			if (window->isOpen())
				return true;
		}

		return false;
	}

	std::shared_ptr<Window> WindowManager::getWindowById(const uint32_t id)
	{
		auto itr = m_Windows.find(id);
		if (itr != m_Windows.end())
		{
			return itr->second;
		}

		return nullptr;
	}

	void WindowManager::foreach(std::function<void(std::shared_ptr<Window>)> func)
	{
		for (auto& [id, window] : m_Windows)
		{
			func(window);
		}
	}
}