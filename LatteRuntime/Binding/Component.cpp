#include "Component.h"
#include "../Rendering/NodeRenderer.h"
#include <nanovg.h>

void latteWidgetDataDeleter(void* usrData)
{
	latte::WidgetData* data = (latte::WidgetData*)usrData;
	delete data;
}

namespace latte
{
	void ComponentSystem::registerComponent(const std::string& name, sol::protected_function construct, const std::string& uiLib)
	{
		if (!m_State)
			return;

		if (!construct.valid()) {
			std::cerr << "Invalid function for component: " << name << std::endl;
			return;
		}

		// Create a wrapper function that automatically adds component_type
        // and stores the original props in the table
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

    // Child processing functions
    static void processChildrenFromTable(LatteNode* node, sol::table childrenTable);
    static LatteNode* findOrCreateChildNode(LatteNode* parent, const std::string& id);
    static void processComponentChild(LatteNode* node, sol::table componentTable);
    static void processRegularChild(LatteNode* node, sol::object childData);

    // Property application functions
    static void applyNodeProperties(LatteNode* node, sol::table table);
    static void applyEventHandlers(WidgetData* data, sol::table table);
    static void applyStyle(WidgetData* data, sol::table table);
    static void applyLayoutProperties(LatteNode* node, WidgetData* data, sol::table table);

    // Widget-specific property functions
    static void applyBoxProperties(LatteNode* node, sol::table table);
    static void applyTextProperties(LatteNode* node, WidgetData* data, sol::table table);

    // Helper functions
    static std::string generateChildId(const std::string& parentId, int childIndex);

    static void processChildrenFromTable(LatteNode* node, sol::table childrenTable)
    {
        for (auto& child : childrenTable)
        {
            std::string childId = generateChildId(node->id, child.first.as<int>());
            ComponentSystem::getInstance().pushID(childId);

            LatteNode* childNode = findOrCreateChildNode(node, childId);

            if (child.second.as<sol::table>()["component_type"].valid())
            {
                processComponentChild(childNode, child.second.as<sol::table>());
            }
            else
            {
                processRegularChild(childNode, child.second);
            }

            ComponentSystem::getInstance().popID();
        }
    }

    static LatteNode* findOrCreateChildNode(LatteNode* parent, const std::string& id)
    {
        // Try to find existing child
        for (int i = 0; i < parent->childCount; i++)
        {
            if (parent->children[i]->id == id)
                return parent->children[i];
        }

        // Create new child
        LatteNode* childNode = latteCreateNode(id.c_str(), parent, LATTE_NODE_FLAGS_DELETE_USERDATA);
        WidgetData* data = new WidgetData;
        latteUserData(childNode, data);
        latteSetUserDataDeleter(childNode, latteWidgetDataDeleter);

        return childNode;
    }

    static void processComponentChild(LatteNode* node, sol::table componentTable)
    {
        ((WidgetData*)latteGetUserData(node))->type = latte::WIDGET_TYPE_BOX;

        std::string componentType = componentTable["component_type"];

        // TODO: Need a better way to handle this
        if (componentType == "Text")
            ((WidgetData*)latteGetUserData(node))->type = latte::WIDGET_TYPE_TEXT;

        sol::table ret = ComponentSystem::getInstance().getComponent(componentType)(componentTable["original_props"]);
        applyPropsFromTable(node, ret);
    }

    static void processRegularChild(LatteNode* node, sol::object childData)
    {
        ((WidgetData*)latteGetUserData(node))->type = latte::WIDGET_TYPE_BOX;
        applyPropsFromTable(node, childData);
    }

    static void applyNodeProperties(LatteNode* node, sol::table table)
    {
        WidgetData* data = (WidgetData*)latteGetUserData(node);
        if (!data) return;

        applyEventHandlers(data, table);
        applyStyle(data, table);
        applyLayoutProperties(node, data, table);
    }

    static void applyEventHandlers(WidgetData* data, sol::table table)
    {
        sol::object obj = table["onPaint"];
        if (obj.valid() && obj.get_type() == sol::type::function)
            data->paint = obj.as<sol::protected_function>();
        else
            data->paint = sol::protected_function();
    }

    static void applyStyle(WidgetData* data, sol::table table)
    {
        sol::object style = table["style"];
        if (style.valid() && style.get_type() == sol::type::table)
            data->style = sol::table(style.as<sol::table>());
    }

    static void applyLayoutProperties(LatteNode* node, WidgetData* data, sol::table table)
    {
        if (data->type == latte::WIDGET_TYPE_BOX)
        {
            applyBoxProperties(node, table);
        }
        else if (data->type == latte::WIDGET_TYPE_TEXT)
        {
            applyTextProperties(node, data, table);
        }
    }

    static void applyBoxProperties(LatteNode* node, sol::table table)
    {
        if (table["padding"].valid() && table["padding"].get_type() == sol::type::table)
        {
            sol::table paddingTable = table["padding"];
            float r = paddingTable.get<float>(1);
            float l = paddingTable.get<float>(2);
            float t = paddingTable.get<float>(3);
            float b = paddingTable.get<float>(4);
            lattePaddingRLTB(node, r, l, t, b);
        }

        if (table["size"].valid() && table["size"].get_type() == sol::type::table)
        {
            sol::table sizeTable = table["size"];
            float w = sizeTable.get<float>(1);
            float h = sizeTable.get<float>(2);
            latteSizer(node, w, h);
        }

        latteSpacing(node, table.get_or("spacing", 0.0f));
        latteMainAxisAlignment(node, (LatteContentAlignment)table.get_or("mainAxisAlignment", (int)LATTE_CONTENT_START));
        latteCrossAxisAlignment(node, (LatteContentAlignment)table.get_or("crossAxisAlignment", (int)LATTE_CONTENT_START));
    }

    static void applyTextProperties(LatteNode* node, WidgetData* data, sol::table table)
    {
        std::string text = table["text"].get<std::string>();

        float fontSize = 14.0f;
        if (data->style.valid())
        {
            fontSize = data->style.get_or("fontSize", 14.0f);
        }

        // TODO: Make this a function in render interface to remove nanovg from this
        nvgFontFace(RenderInterface::getInstance().getNVGContext(), "Roboto-Regular");
        nvgFontSize(RenderInterface::getInstance().getNVGContext(), fontSize);

        float bounds[4];
        float w = nvgTextBounds(RenderInterface::getInstance().getNVGContext(), 0.0f, 0.0f, text.c_str(), NULL, bounds);
        float h = bounds[3] - bounds[1];

        data->text = text;
        latteSizer(node, w, h);
    }

    static std::string generateChildId(const std::string& parentId, int childIndex)
    {
        return parentId + "/" + std::to_string(childIndex);
    }

    void applyPropsFromTable(LatteNode* node, sol::table table, bool applyForThis)
    {
        printf("Laying out node from Lua: %s\n", node->id);

        if (table["children"].valid() && table["children"].get_type() == sol::type::table)
        {
            processChildrenFromTable(node, table["children"]);
        }

        if (applyForThis)
        {
            applyNodeProperties(node, table);
        }
    }
}