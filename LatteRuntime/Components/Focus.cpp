#include "Focus.h"
#include "Component.h"

namespace latte
{
	Focus::Focus(const std::string& id) : m_ID(id)
	{
	}

	bool Focus::isFocused()
	{
		LatteNode* focusedNode = latte::ComponentSystem::getInstance().getFocusedNode();
		if (focusedNode == nullptr)
			return false;

		return std::string(focusedNode->id) == m_ID;
	}

	void Focus::request()
	{
		if (m_ID.empty())
			return;

		LatteNode* node = latte::ComponentSystem::getInstance().findNode(m_ID);

		if (!node)
			return;

		latte::ComponentSystem::getInstance().setFocusedNode(node);
		latte::Log::log(latte::Log::Severity::Info, "Set node has focus: {}", m_ID);
	}

	void Focus::luaRegister(sol::state_view state)
	{
		state.new_usertype<latte::Focus>("Focus",
			"isFocused", &latte::Focus::isFocused,
			"request", &latte::Focus::request
		);
	}
}