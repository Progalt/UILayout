#ifndef LATTE_WINDOWMANAGER_H
#define LATTE_WINDOWMANAGER_H

#include <memory>
#include <cstdint>
#include <map>
#include <functional>

#include "../Utils/Singleton.h"
#include "Window.h"

namespace latte
{
	class WindowManager
	{
	public:

		void registerWindow(std::shared_ptr<Window> window);

		bool isSomeWindowOpen();

		std::shared_ptr<Window> getWindowById(const uint32_t id);

		void foreach(std::function<void(std::shared_ptr<Window>)> func);

		void removeWindow(uint32_t id);

	private:


		std::map<uint32_t, std::shared_ptr<Window>> m_Windows;
	};
}

#endif // LATTE_WINDOWMANAGER_H