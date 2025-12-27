#include "ComponentEvents.h"
extern "C" {
#include <LatteLayout/layout.h>
}
#include "Component.h"
#include <variant>
#include "../Utils/Log.h"

namespace latte
{
    bool handleNodeEvent(Event evnt, LatteNode* node)
    {
        bool handled = false;
        for (int i = 0; i < node->childCount; i++)
        {
            handled = handleNodeEvent(evnt, node->children[i]);
            if (handled)
                break;
        }

        ComponentData* compData = (ComponentData*)latteGetUserData(node);
        ComponentState& state = compData->internalState;
        float boundingBox[4];
        latteGetScreenBoundingBox(node, boundingBox);

        

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
                        {
                            // Entered
                            //Log::log(Log::Severity::Info, "Mouse Entered {}", node->id);

                            auto itr = compData->eventCallbacks.find(COMPONENT_EVENT_HOVER_ENTER);
                            if (itr != compData->eventCallbacks.end())
                                itr->second();
                        }

                        state.hovered = true;
                        handled = true;
                    }
                    else
                    {
                        if (state.hovered)
                        {
                            // Exited
                            //Log::log(Log::Severity::Info, "Mouse Exited {}", node->id);

                            auto itr = compData->eventCallbacks.find(COMPONENT_EVENT_HOVER_EXIT);
                            if (itr != compData->eventCallbacks.end())
                                itr->second();
                        }

                        state.hovered = false;
                    }
                }
                else if constexpr (std::is_same_v<T, MouseButtonEvent>)
                {
                    // TODO: handle MouseButtonEvent
                }
                }, evnt);
        }
        return handled;
    }
}