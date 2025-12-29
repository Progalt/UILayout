
#ifndef LATTE_COMPONENT_H
#define LATTE_COMPONENT_H

#include <sol/sol.hpp>
extern "C" {
#include <LatteLayout/layout.h>
}
#include "../Utils/Singleton.h"
#include <unordered_map>
#include <stack>
#include "ComponentLibrary.h"
#include "../Utils/Log.h"

namespace latte
{
	enum WidgetType
	{
		WIDGET_TYPE_BOX,
		WIDGET_TYPE_TEXT
	};

	struct ComponentState
	{
		bool hovered;
		bool leftDown;
	};

	enum ComponentEvent
	{
		COMPONENT_EVENT_PAINT,
		COMPONENT_EVENT_HOVER_ENTER,
		COMPONENT_EVENT_HOVER_EXIT,
		COMPONENT_EVENT_CLICK, 
		COMPONENT_EVENT_KEY_DOWN,
		COMPONENT_EVENT_TEXT_INPUT
	};

	// This is a struct that gets attached to the user data of the node
	struct ComponentData
	{
		WidgetType type;
		sol::table style;

		sol::table state;

		std::unordered_map<ComponentEvent, sol::protected_function> eventCallbacks;

		// For text widgets 
		std::string text;
		float fontSize;

		ComponentState internalState;
	};


	class ComponentSystem : public Singleton<ComponentSystem>
	{
	public:

		void setState(sol::state* s) { m_State = s; }

		// void registerComponent(const std::string& name, sol::protected_function construct, const std::string& uiLib);

		sol::protected_function getComponent(const std::string& name);

		void pushID(const std::string& id) { m_IdStack.push(id); }
		void popID() { m_IdStack.pop(); }

		const std::string getCurrentID() const { 
			if (m_IdStack.size() == 0)
				return std::string("");

			return m_IdStack.top(); 
		}

		LatteNode* findNode(const std::string& id);

		void setFocusedNode(LatteNode* node)
		{
			m_FocusedNode = node;
		}

		LatteNode* getFocusedNode()
		{
			return m_FocusedNode;
		}

		std::shared_ptr<latte::ComponentLibrary> createComponentLibrary(const std::string& name)
		{
			auto itr = m_Libraries.find(name);
			if (itr != m_Libraries.end())
			{
				Log::log(Log::Severity::Warning, "Component library {} already exists, just returning that.", name);
			}

			Log::log(Log::Severity::Info, "Creating component library: {}", name);

			std::shared_ptr<latte::ComponentLibrary> lib = std::make_shared<latte::ComponentLibrary>(*m_State, name);
			m_Libraries[name] = lib;

			return lib;
		}

	private:

		std::unordered_map<std::string, sol::protected_function> m_Components;

		std::unordered_map<std::string, std::shared_ptr<latte::ComponentLibrary>> m_Libraries;

		sol::state* m_State;

		std::stack<std::string> m_IdStack;

		LatteNode* m_FocusedNode = nullptr;


		
	};

	void applyPropsFromTable(LatteNode* node, sol::table table, bool applyForThis = true);
}

#endif // LATTE_COMPONENT_H