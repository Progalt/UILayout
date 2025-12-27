
#ifndef LATTE_EVENT_H
#define LATTE_EVENT_H

#include <variant>
#include <cstdint>

namespace latte
{
	enum class MouseButton : uint8_t
	{
		Left = 1,
		Middle = 2,
		Right = 3
	};

	enum class ButtonState : uint8_t
	{
		Down, 
		Up
	};

	struct MouseMotionEvent
	{
		int x, y;
		int dx, dy;
	};

	struct MouseButtonEvent
	{
		MouseButton button;
		ButtonState state;
	};

	using Event = std::variant<MouseMotionEvent, MouseButtonEvent>;
}

#endif // LATTE_EVENT_H