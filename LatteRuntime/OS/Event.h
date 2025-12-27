
#ifndef LATTE_EVENT_H
#define LATTE_EVENT_H

#include <variant>

namespace latte
{
	struct MouseMotionEvent
	{
		int x, y;
		int dx, dy;
	};

	struct MouseButtonEvent
	{
		int button;
	};

	using Event = std::variant<MouseMotionEvent, MouseButtonEvent>;
}

#endif // LATTE_EVENT_H