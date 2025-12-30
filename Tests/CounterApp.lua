
-- create a router
local router = Router.new()
local appUI = latte.createComponentLibrary("appUI")

appUI:register("Counter", function(props)

    local state = latte.useState({
        count = 0
    })

    return latte.ui.Container({
        mainAxisAlignment = latte.contentAlignment.center,
        crossAxisAlignment = latte.contentAlignment.center,
        size = { latte.size.grow, latte.size.grow },
        style = {
            backgroundColor = latte.color.hex("#fafafa"),
        },
        children = {
            latte.ui.VBox({
                spacing = 16,
                mainAxisAlignment = latte.contentAlignment.center,
                crossAxisAlignment = latte.contentAlignment.center,
                children = {
                    latte.ui.Text({
                        text = "Press the button to increment the counter below.",
                        style = latte.mergeStyles({
                            fontSize = 14,
                            color = latte.color.hex("#555555")
                        }, props.subtitleStyle or {})
                    }),
                    latte.ui.Text({
                        text = "Count: " .. tostring(state.count),
                        style = latte.mergeStyles({
                            fontSize = 24,
                            color = latte.color.hex("#333333")
                        }, props.textStyle or {})
                    }),
                    latte.fluent.Button({
                        "Press Me!",
                        onClick = function()

                            if state.count + 1 >= 10 then
                                router:navigate("/finish")
                                return
                            end

                            state:setState({ count = state.count + 1 })
                            
                        end
                    }),
                    latte.ui.TextField({
                        placeholder = "Type something...",
                        style = {
                            width = 200,
                        }
                    })
                }
            }),
            
        }
    })
end)

function home()
    return {
        children = {
            latte.appUI.Counter({})
        }
    }
end

function finish()
    return {
        children = {
            latte.ui.Container({
                mainAxisAlignment = latte.contentAlignment.center,
                crossAxisAlignment = latte.contentAlignment.center,
                size = { latte.size.grow, latte.size.grow },
                spacing = 16, 
                style = {
                    backgroundColor = latte.color.hex("#fafafa"),
                },
                children = {
                    latte.ui.Text({
                        text = "Thank you for using the Counter App!",
                        style = {
                            fontSize = 18,
                            color = latte.color.hex("#333333")
                        }
                    }),
                    latte.fluent.Button({
                        "Back!",
                        onClick = function()
                            router:back()
                        end
                    }),
                }
            })
        }
    }
end

-- Define routes
router:define("/home", home)
router:define("/finish", finish)

-- Since useRouter hasn't been called, this tells the router to start here when it is
router:navigate("/home")

-- Set window properties for the router
router:setWindowData({
    title = "Counter",
    size = { 400, 300 },
})

-- Creates a window and assigns it to the router
latte.useRouter(router)
