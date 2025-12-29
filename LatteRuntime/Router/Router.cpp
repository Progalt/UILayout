#include "Router.h"
#include <regex>
#include "../Utils/Log.h"
#include "../OS/EventLoop.h"

namespace latte
{
	Router::Router()
	{

	}

	void Router::define(const std::string& routeName, sol::protected_function builder)
	{
		m_Routes[routeName] = Route(routeName, builder);
	}

	void Router::navigate(const std::string& routeName, bool replace)
	{
		Log::log(Log::Severity::Info, "Pushing Route: {}", routeName);

		std::string pattern = matchStatic(routeName);
		if (pattern.empty()) 
		{
			Log::log(Log::Severity::Warning, "No Route Found: {}", routeName);
			return;
		}

		if (replace && !m_RouteStack.empty()) 
		{
			m_RouteStack.pop();
		}

		ActiveRoute route{};
		route.concretePath = pattern;
		route.pattern = pattern;
		route.parameters = sol::nil;
		m_RouteStack.push(route);

		if (m_Window)
		{
			sol::protected_function builder = getRouteFunction(routeName);
			sol::object obj = builder();
			if (obj.get_type() != sol::type::table)
			{
				latte::Log::log(latte::Log::Severity::Warning, "Route Builder must return a valid table: {}", route.pattern);
				return;
			}
			
			m_Window->setLuaRootTable(obj.as<sol::table>());

			EventLoop::getInstance().pushRelayout(m_Window);
		}
	}

	void Router::back()
	{
		if (!m_RouteStack.empty()) 
		{
			m_RouteStack.pop();
			if (!m_RouteStack.empty()) 
			{
				
			}
		}
	}

	void Router::luaRegister(sol::state_view state)
	{
		state.new_usertype<latte::Router>("Router",
			sol::constructors<latte::Router()>(),
			"define", &latte::Router::define,
			"navigate", &latte::Router::navigate,
			"back", &latte::Router::back
		);
	}

	std::string Router::matchStatic(const std::string& path)
	{
		for (const auto& [pattern, _] : m_Routes) 
		{
			if (pattern == path) {
				return pattern; 
			}
		}
		return "";
	}
}