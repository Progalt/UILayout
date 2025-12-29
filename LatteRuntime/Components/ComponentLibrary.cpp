
#include "ComponentLibrary.h"
#include "../Utils/Log.h"

namespace latte
{
	ComponentLibrary::ComponentLibrary(sol::state_view state, const std::string& name) : m_Name(name)
	{
		sol::object latteObj = state["latte"];
		if (!latteObj.is<sol::table>()) 
		{
			throw std::runtime_error("'latte' table not found in Lua state! Cannot create component library.");
		}
		sol::table latteTable = latteObj.as<sol::table>();

		sol::object compObj = latteTable[name];
		if (compObj.is<sol::table>()) {
			m_ComponentTable = compObj.as<sol::table>();
		}
		else {
			m_ComponentTable = latteTable.create_named(name);
		}
	}

	void ComponentLibrary::registerComponent(const std::string& name, sol::protected_function builder)
	{
		if (m_Components.find(name) != m_Components.end())
		{
			Log::log(Log::Severity::Warning, "Component with name {} already registered. So ignoring...", name);
			return;
		}

		auto wrapper = [this, name, builder](sol::table props) -> sol::object
			{
				// Call the original component function
				sol::protected_function_result result = builder(props);

				if (!result.valid())
				{
					sol::error err = result;
					Log::log(Log::Severity::Error, "Component {} failed: {}", name, err.what());
					return sol::nil;
				}

				sol::table componentTable = result;
				componentTable["component_type"] = m_Name + "." + name;
				componentTable["original_props"] = props;

				return componentTable;
			};

		m_Components[name] = builder;

		m_ComponentTable[name] = wrapper;
		Log::log(Log::Severity::Info, "Registered Component: {} in Component Library: {}", name, m_Name);
	}

	sol::protected_function ComponentLibrary::getComponent(const std::string& name) const
	{
		auto itr = m_Components.find(name);
		if (itr != m_Components.end())
			return itr->second;

		return sol::nil;
	}

	void ComponentLibrary::luaRegister(sol::state_view state)
	{
		state.new_usertype<latte::ComponentLibrary>("ComponentLibrary",
			"register", &latte::ComponentLibrary::registerComponent
		);
	}
}