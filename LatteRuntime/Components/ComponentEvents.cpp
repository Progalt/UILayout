#include "ComponentEvents.h"
extern "C" {
#include <LatteLayout/layout.h>
}
#include "Component.h"
#include <variant>
#include "../Utils/Log.h"
#include "../OS/EventLoop.h"

namespace latte
{
    bool handleNodeEvent(Event evnt, LatteNode* node, sol::state_view luaState)
    {
        bool handled = false;
        for (int i = 0; i < node->childCount; i++)
        {
            handled = handleNodeEvent(evnt, node->children[i], luaState);
            if (handled)
                break;
        }

        ComponentData* compData = (ComponentData*)latteGetUserData(node);

        if (compData == nullptr)
            return false;

        ComponentState& state = compData->internalState;
        float boundingBox[4];
        latteGetScreenBoundingBox(node, boundingBox);

        // Helper so this logic isn't duplicated for every event
        auto passEvent = 
            [&](latte::ComponentEvent compEvnt, auto&&... args) -> bool {
            auto itr = compData->eventCallbacks.find(compEvnt);
            if (itr != compData->eventCallbacks.end())
            {
                itr->second(std::forward<decltype(args)>(args)...);
                return true;
            }

            return false;
        };

        if (!handled)  // Only do for parent if children didn't handle
        {
            std::visit([&](const auto& e) {
                using T = std::decay_t<decltype(e)>;
                if constexpr (std::is_same_v<T, MouseMotionEvent>)
                {
                    float x = static_cast<float>(e.x);
                    float y = static_cast<float>(e.y);

                    if (x >= boundingBox[0] && x < boundingBox[2] &&
                        y >= boundingBox[1] && y < boundingBox[3])
                    {
                        if (!state.hovered)
                            passEvent(COMPONENT_EVENT_HOVER_ENTER, true );

                        state.hovered = true;
                        handled = true;
                    }
                    else
                    {
                        if (state.hovered)
                            passEvent(COMPONENT_EVENT_HOVER_EXIT);

                        state.hovered = false;
                    }
                }
                else if constexpr (std::is_same_v<T, MouseButtonEvent>)
                {
                    // TODO: handle MouseButtonEvent

                    if (e.button == MouseButton::Left)
                    {
                        if (e.state == ButtonState::Down && state.hovered)
                            state.leftDown = true;

                        if (e.state == ButtonState::Up)
                        {

                            if (state.hovered && state.leftDown)
                            {
                                sol::table exData = luaState.create_table();
                                exData["x"] = e.x - boundingBox[0];
                                exData["y"] = e.y - boundingBox[1];

                                bool removeFocus = true;
                                if (passEvent(COMPONENT_EVENT_CLICK, exData))
                                {
                                    Log::log(Log::Severity::Info, "Clicked Node: {}", std::string(node->id));

                                    // Kind of bad way of removing focus
                                    // Remove focus if the clicked node does not equal the focused node
                                    if (ComponentSystem::getInstance().getFocusedNode() == node)
                                    {
                                        removeFocus = false;
                                    }

                                    handled = true;
                                }

                                if (removeFocus)
                                {
                                    ComponentSystem::getInstance().setFocusedNode(nullptr);

                                    // Trigger a relayout
                                    // Very scuffed needs a fix
                                    latte::EventLoop::getInstance().getWindowManager().foreach([&](std::shared_ptr<latte::Window> win) {
                                        latte::EventLoop::getInstance().pushRelayout(win);
                                    });
                                }

                                
                            }

                            state.leftDown = false;

                        }

                    }
                }
                else if constexpr (std::is_same_v<T, KeyDownEvent>)
                {
                    sol::table keyMod = luaState.create_table();

                    for (const auto& mod : e.keyMods) {
                        if (mod == "left shift")    keyMod["leftShift"] = true;
                        else if (mod == "right shift")   keyMod["rightShift"] = true;
                        else if (mod == "left ctrl")    keyMod["leftCtrl"] = true;
                        else if (mod == "right ctrl")    keyMod["rightCtrl"] = true;
                        else if (mod == "left alt")     keyMod["leftAlt"] = true;
                        else if (mod == "right alt")     keyMod["rightAlt"] = true;
                        else if (mod == "left gui")     keyMod["leftGui"] = true;
                        else if (mod == "right gui")     keyMod["rightGui"] = true;
                       /* else if (mod == "num")      keyMod["num"] = true;
                        else if (mod == "caps")     keyMod["caps"] = true;
                        else if (mod == "mode")     keyMod["mode"] = true;*/
                    }

                    if (passEvent(COMPONENT_EVENT_KEY_DOWN, e.name, keyMod))
                        handled = true;
                }
                else if constexpr (std::is_same_v<T, TextInputEvent>)
                {
                    if (passEvent(COMPONENT_EVENT_TEXT_INPUT, e.str))
                        handled = true;
                }
                }, evnt);
        }
        return handled;
    }
}