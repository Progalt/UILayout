
#ifndef LATTE_ROUTER_H
#define LATTE_ROUTER_H

#include <stack>
#include <string>
#include <sol/sol.hpp>
#include "../OS/Window.h"

namespace latte
{
	struct Route
	{
		Route() : pattern(""), builder(sol::nil) {}
		Route(const std::string& p, const sol::protected_function& b)
			: pattern(p), builder(b) { }

		std::string pattern;
		sol::protected_function builder;
	};

	struct ActiveRoute
	{
		std::string pattern;
		std::string concretePath;
		sol::table parameters;
	};

	/*
		A router allows you to use web like URLs for navigation and passing data

		Like a route of
		/home
		/user/[id] which could be /user/879 

		A route is a stack  
	*/
	class Router
	{
	public:

		Router();

		/*
			Define a new route
			Takes a function to call when (re)building the route
		*/
		void define(const std::string& routeName, sol::protected_function builder);

		/*
			Navigate to the specified route

			When replace is true it will pop from the route stack and add this. 
			Otherwise it will just push this route if it exists
		*/
		void navigate(const std::string& routeName, bool replace = false);

		/*
			Pop off the stack and go back

			On certain platforms this will either close the app or close the window. 
			On platforms with multiple windows, this will be this window. It might close the app if it is the host window
		*/
		void back();

		static void luaRegister(sol::state_view state);

		const ActiveRoute& getActiveRoute() const { return m_RouteStack.top(); }

		sol::protected_function getRouteFunction(const std::string& pattern) const
		{
			auto itr = m_Routes.find(pattern);
			if (itr != m_Routes.end())
			{
				return itr->second.builder;
			}

			return sol::nil;
		}

		void setWindow(std::shared_ptr<Window> win)
		{
			m_Window = win;
		}

	private:

		/*
			Helper function to match static routes
		*/
		std::string matchStatic(const std::string& path);

		std::stack<ActiveRoute> m_RouteStack;

		std::unordered_map<std::string, Route> m_Routes;

		std::shared_ptr<Window> m_Window;
	};
}

#endif // LATTE_ROUTER_H