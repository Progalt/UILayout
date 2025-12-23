
#ifndef LATTE_COMPONENT_H
#define LATTE_COMPONENT_H

#include <sol/sol.hpp>
extern "C" {
#include <LatteLayout/layout.h>
}
#include "../Utils/Singleton.h"
#include <unordered_map>
#include <stack>

namespace latte
{
	enum WidgetType
	{
		WIDGET_TYPE_BOX,
		WIDGET_TYPE_TEXT
	};

	// This is a struct that gets attached to the user data of the node
	struct WidgetData
	{
		WidgetType type;
		sol::protected_function paint;
		sol::table style;
	};


	class ComponentSystem : public Singleton<ComponentSystem>
	{
	public:

		void setState(sol::state* s) { m_State = s; }

		void registerComponent(const std::string& name, sol::protected_function construct);

		sol::protected_function getComponent(const std::string& name);

		void pushID(const std::string& id) { m_IdStack.push(id); }
		void popID() { m_IdStack.pop(); }

		const std::string getCurrentID() const { 
			if (m_IdStack.size() == 0)
				return std::string("");

			return m_IdStack.top(); 
		}

	private:

		std::unordered_map<std::string, sol::protected_function> m_Components;

		sol::state* m_State;

		std::stack<std::string> m_IdStack;

		
	};

	void applyPropsFromTable(LatteNode* node, sol::table table, bool applyForThis = true);
}

#endif // LATTE_COMPONENT_H