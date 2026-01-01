#include "Component.h"
#include "../Rendering/NodeRenderer.h"
#include <nanovg.h>
#include "../Utils/Log.h"
#include "../OS/EventLoop.h"
#include <set>

void latteWidgetDataDeleter(void* usrData)
{
	latte::ComponentData* data = (latte::ComponentData*)usrData;
	delete data;
}

namespace latte
{
	sol::protected_function ComponentSystem::getComponent(const std::string& name)
	{

        int firstDot = name.find_first_of('.');
        std::string lib = name.substr(0, firstDot);
        std::string compName = name.substr(firstDot + 1);

        auto libItr = m_Libraries.find(lib);
        if (libItr == m_Libraries.end())
        {
            Log::log(Log::Severity::Error, "Could not find Component {} in Component Library {} -> Library doesn't exist ", compName, lib);
            return sol::nil;
        }

        return libItr->second->getComponent(compName);

	}

    LatteNode* findLatteNode(const std::string& id, LatteNode* node)
    {
        if (std::string(node->id) == id)
            return node;

        for (int i = 0; i < node->childCount; i++)
        {
            LatteNode* val = findLatteNode(id, node->children[i]);

            if (val)
                return val;
        }

        return nullptr;
    }

    LatteNode* ComponentSystem::findNode(const std::string& id)
    {
        LatteNode* result = nullptr;
        WindowManager& winMgr = EventLoop::getInstance().getWindowManager();
        EventLoop::getInstance().getWindowManager().foreach(
            [&](std::shared_ptr<Window> win) {
                if (!result)
                    result = findLatteNode(id, win->getRootNode());
            }
        );
        return result;
    }

    // Child processing functions
    static void processChildrenFromTable(LatteNode* node, sol::table childrenTable);
    static LatteNode* findOrCreateChildNode(LatteNode* parent, const std::string& id);
    static void processComponentChild(LatteNode* node, sol::table componentTable);
    static void processRegularChild(LatteNode* node, sol::object childData);

    // Property application functions
    static void applyNodeProperties(LatteNode* node, sol::table table);
    static void applyEventHandlers(ComponentData* data, sol::table table);
    static void applyStyle(ComponentData* data, sol::table table);
    static void applyLayoutProperties(LatteNode* node, ComponentData* data, sol::table table);

    // Widget-specific property functions
    static void applyBoxProperties(LatteNode* node, sol::table table);
    static void applyTextProperties(LatteNode* node, ComponentData* data, sol::table table);

    // Helper functions
    static std::string generateChildId(const std::string& parentId, int childIndex, sol::table table = sol::nil);

    static void processChildrenFromTable(LatteNode* node, sol::table childrenTable)
    {
        std::set<std::string> childrenToKeep;
        std::set<std::string> newIds;

        // Collect all new child IDs
        for (auto& child : childrenTable)
        {
            std::string childId = generateChildId(node->id, child.first.as<int>(), child.second.as<sol::table>());
            newIds.insert(childId);
        }

        // Identify and remove obsolete children BEFORE building new ones
        std::vector<LatteNode*> toRemove;
        for (int i = 0; i < node->childCount; ++i)
        {
            if (newIds.find(node->children[i]->id) == newIds.end())
            {
                toRemove.push_back(node->children[i]);
            }
        }
        // Actually remove them
        for (LatteNode* doomed : toRemove)
        {
            Log::log(Log::Severity::Info, "Remove node: {}", doomed->id);
            latteFreeNode(doomed);
        }

        // Now process the new layout
        for (auto& child : childrenTable)
        {
            std::string childId = generateChildId(node->id, child.first.as<int>(), child.second.as<sol::table>());
            ComponentSystem::getInstance().pushID(childId);
            childrenToKeep.insert(childId);

            LatteNode* childNode = findOrCreateChildNode(node, childId);
            ((ComponentData*)latteGetUserData(childNode))->effectOffset = 0;

            if (child.second.as<sol::table>()["component_type"].valid())
                processComponentChild(childNode, child.second.as<sol::table>());
            else
                processRegularChild(childNode, child.second);

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
        ComponentData* data = new ComponentData;
        memset(&data->internalState, 0, sizeof(ComponentState));
        latteUserData(childNode, data);
        latteSetUserDataDeleter(childNode, latteWidgetDataDeleter);

        return childNode;
    }

    static void processComponentChild(LatteNode* node, sol::table componentTable)
    {
        ((ComponentData*)latteGetUserData(node))->type = latte::WIDGET_TYPE_BOX;

        std::string componentType = componentTable["component_type"];

        // TODO: Need a better way to handle this
        if (componentType == "ui.Text")
            ((ComponentData*)latteGetUserData(node))->type = latte::WIDGET_TYPE_TEXT;

        sol::protected_function ctor = ComponentSystem::getInstance().getComponent(componentType);
        if (!ctor.valid()) 
        {
            Log::log(Log::Severity::Error, "No component found for type '{}'", componentType);
            return; 
        }

        sol::protected_function_result result = ctor(componentTable["original_props"]);
        if (!result.valid()) 
        {
            sol::error err = result;
            Log::log(Log::Severity::Error, "Component {} failed: {}", componentType, err.what());
            return;
        }

        sol::table ret = result;
        applyPropsFromTable(node, ret);
    }

    static void processRegularChild(LatteNode* node, sol::object childData)
    {
        ((ComponentData*)latteGetUserData(node))->type = latte::WIDGET_TYPE_BOX;
        applyPropsFromTable(node, childData);
    }

    static void applyNodeProperties(LatteNode* node, sol::table table)
    {
        ComponentData* data = (ComponentData*)latteGetUserData(node);
        if (!data) return;

        applyEventHandlers(data, table);
        applyStyle(data, table);
        applyLayoutProperties(node, data, table);
    }

    static void applyEventHandlers(ComponentData* data, sol::table table)
    {
        auto getEventHandler = [&](const std::string& luaName, ComponentEvent evnt)
            {
                sol::object obj = table[luaName];
                if (obj.valid() && obj.get_type() == sol::type::function)
                    data->eventCallbacks[evnt] = obj.as<sol::protected_function>();
            };

        getEventHandler("onPaint", COMPONENT_EVENT_PAINT);
        getEventHandler("onHoverEnter", COMPONENT_EVENT_HOVER_ENTER);
        getEventHandler("onHoverExit", COMPONENT_EVENT_HOVER_EXIT);
        getEventHandler("onClick", COMPONENT_EVENT_CLICK);
        getEventHandler("onKeyDown", COMPONENT_EVENT_KEY_DOWN);
        getEventHandler("onTextInput", COMPONENT_EVENT_TEXT_INPUT);
    }

    static void applyStyle(ComponentData* data, sol::table table)
    {
        sol::object style = table["style"];
        if (style.valid() && style.get_type() == sol::type::table)
            data->style = sol::table(style.as<sol::table>());
    }

    static void applyLayoutProperties(LatteNode* node, ComponentData* data, sol::table table)
    {
        if (data->type == latte::WIDGET_TYPE_BOX)
        {
            applyBoxProperties(node, table);
        }
        else if (data->type == latte::WIDGET_TYPE_TEXT)
        {
            applyTextProperties(node, data, table);
        }

        // Handle getting the positioner stuff
        // Layout like this is the same for text and box model stuff
        int positioner = table.get_or("layout", 0);
        if (positioner == LATTE_POSITIONER_ABSOLUTE)
        {
            float reqx = 0.0f, reqy = 0.0f;

            sol::object positionTable = table["position"];
            if (positionTable.get_type() == sol::type::table)
            {
                sol::table t = positionTable.as<sol::table>();
                reqx = t.get_or(1, 0.0f);
                reqy = t.get_or(2, 0.0f);
            }

            latteAbsolutePositioner(node, reqx, reqy);
        }
    }

    static void applyBoxProperties(LatteNode* node, sol::table table)
    {
        if (table["padding"].valid() && table["padding"].get_type() == sol::type::table)
        {
            sol::table paddingTable = table["padding"];
            float l = paddingTable.get<float>(1);
            float t = paddingTable.get<float>(2);
            float r = paddingTable.get<float>(3);
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

        std::string dir = table.get_or("direction", std::string("horizontal"));
        if (dir == "horizontal")
            latteMainAxisDirection(node, LATTE_DIRECTION_HORIZONTAL);
        else if (dir == "vertical")
            latteMainAxisDirection(node, LATTE_DIRECTION_VERTICAL);
    }

    static void applyTextProperties(LatteNode* node, ComponentData* data, sol::table table)
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

    static std::string generateChildId(const std::string& parentId, int childIndex, sol::table table)
    {
        // Early out if valid string id
        if (table != sol::nil) 
        {
            sol::object idObj = table["id"];
            if (idObj.is<std::string>()) 
            {
                return parentId + "/" + idObj.as<std::string>();
            }

            sol::object compTypeObj = table["component_type"];
            if (compTypeObj.is<std::string>()) 
            {
                return parentId + "/" + std::to_string(childIndex) +
                    "." + compTypeObj.as<std::string>();
            }
        }
        // Fallback if no component_type found or table is nil
        return parentId + "/" + std::to_string(childIndex);
    }

    void applyPropsFromTable(LatteNode* node, sol::table table, bool applyForThis)
    {

        // Log::log(Log::Severity::Info, "Rebuilding Node: {}", node->id);

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