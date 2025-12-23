#include "Component.h"

void latteWidgetDataDeleter(void* usrData)
{
	latte::WidgetData* data = (latte::WidgetData*)usrData;
	delete data;
}

namespace latte
{
	void ComponentSystem::registerComponent(const std::string& name, sol::protected_function construct)
	{
		if (!m_State)
			return;

		if (!construct.valid()) {
			std::cerr << "Invalid function for component: " << name << std::endl;
			return;
		}

		// Create a wrapper function that automatically adds component_type
		auto wrapper = [this, name, construct](sol::table props) -> sol::object 
		{
			// Call the original component function
			sol::protected_function_result result = construct(props);

			if (!result.valid()) 
			{
				sol::error err = result;
				std::cerr << "Component " << name << " failed: " << err.what() << std::endl;
				return sol::nil;
			}

			sol::table componentTable = result;
			componentTable["component_type"] = name;
			componentTable["original_props"] = props;

			return componentTable;
		};

		m_Components[name] = construct;

		sol::table latteTable = (*m_State)["latte"];
		sol::table uiTable = latteTable["ui"];

		// Assign the wrapped component
		uiTable[name] = wrapper;
		std::cout << "Registered component: " << name << std::endl;
	}

	sol::protected_function ComponentSystem::getComponent(const std::string& name)
	{
		if (!m_State)
			return sol::nil;

		auto itr = m_Components.find(name);
		if (itr != m_Components.end())
		{
			return itr->second;
		}

		return sol::nil;
	}

	void applyPropsFromTable(LatteNode* node, sol::table table, bool applyForThis)
	{
		if (applyForThis)
		{
			latteSpacing(node, table.get_or("spacing", 0.0f));

			if (table["padding"].valid() && table["padding"].get_type() == sol::type::table)
			{
				sol::table paddingTable = table["padding"];

				float r, l, t, b;
				r = paddingTable.get<float>(1);
				l = paddingTable.get<float>(2);
				t = paddingTable.get<float>(3);
				b = paddingTable.get<float>(4);

				lattePaddingRLTB(node, r, l, t, b);
			}

			if (table["size"].valid() && table["size"].get_type() == sol::type::table)
			{
				sol::table sizeTable = table["size"];

				float w, h;
				w = sizeTable.get<float>(1);
				h = sizeTable.get<float>(2);

				latteSizer(node, w, h);
			}

			WidgetData* data = (WidgetData*)latteGetUserData(node);
			if (data)
			{
				sol::object obj = table["onPaint"];
				if (obj.valid() && obj.get_type() == sol::type::function) 
				{
					data->paint = obj.as<sol::protected_function>();
				}
				else {
					// handle error: maybe log, or assign a nullptr
					printf("BFUFUUFUFUFUF\n");
					data->paint = sol::protected_function();
				}

				sol::object style = table["style"];
				if (style.valid() && style.get_type() == sol::type::table)
				{
					data->style = sol::table(style.as<sol::table>());
				}
			}
		}

		if (table["children"].valid() && table["children"].get_type() == sol::type::table)
		{
			sol::table childrenTable = table["children"];

			for (auto& child : childrenTable)
			{
				std::string id = node->id;
				id += "/" + std::to_string(child.first.as<int>());

				ComponentSystem::getInstance().pushID(id);

				LatteNode* childNode = nullptr;
				
				// Try to find the node if it exists already
				for (int i = 0; i < node->childCount; i++)
				{
					std::string tid = node->children[i]->id;
					if (tid == id)
						childNode = node->children[i];
				}

				if (childNode == nullptr)
				{
					childNode = latteCreateNode(id.c_str(), node, LATTE_NODE_FLAGS_DELETE_USERDATA);
					WidgetData* data = new WidgetData;
					latteUserData(childNode, data);
					latteSetUserDataDeleter(childNode, latteWidgetDataDeleter);
					
				}

				// If it has a component_type, it is a component generated with a function
				// So regen it
				if (child.second.as<sol::table>()["component_type"].valid())
				{
					((WidgetData*)latteGetUserData(childNode))->type = latte::WIDGET_TYPE_BOX;

					sol::table t = child.second.as<sol::table>();
					std::string componentType = t["component_type"];
					sol::table ret = ComponentSystem::getInstance().getComponent(componentType)(t["original_props"]);

					applyPropsFromTable(childNode, ret);
				}
				else
				{
					((WidgetData*)latteGetUserData(childNode))->type = latte::WIDGET_TYPE_BOX;

					applyPropsFromTable(childNode, child.second);
				}

				ComponentSystem::getInstance().popID();
			}
		}
	}
}