
#ifndef LATTE_COMPONENT_LIBRARY_H
#define LATTE_COMPONENT_LIBRARY_H

#include <sol/sol.hpp>
#include <string>

namespace latte
{
	class ComponentLibrary
	{
	public:

		ComponentLibrary(sol::state_view state, const std::string& name);

		void registerComponent(const std::string& name, sol::protected_function builder);

		[[nodiscard]] sol::protected_function getComponent(const std::string& name) const;

		static void luaRegister(sol::state_view state);

		[[nodiscard]] const std::string& getName() const noexcept { return m_Name; }

	private:

		sol::table m_ComponentTable;
		std::string m_Name = "";
		std::unordered_map<std::string, sol::protected_function> m_Components;
	};
}

#endif // LATTE_COMPONENT_LIBRARY_H