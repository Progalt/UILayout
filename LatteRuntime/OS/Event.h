
#ifndef LATTE_EVENT_H
#define LATTE_EVENT_H

#include <variant>
#include <cstdint>
#include <string>

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

	struct KeyDownEvent
	{
		int keyCode;
		std::string name;
	};

	struct MouseButtonEvent
	{
		MouseButton button;
		ButtonState state;
	};

	struct TextInputEvent
	{
		std::string str;
	};

	using Event = std::variant<MouseMotionEvent, MouseButtonEvent, KeyDownEvent, TextInputEvent>;
}

#endif // LATTE_EVENT_H