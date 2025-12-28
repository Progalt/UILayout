

latte.registerComponent("Counter", function(props)

    local state = latte.useState({
        count = 0
    })

    return latte.ui.Container({
        mainAxisAlignment = latte.contentAlignment.center,
        crossAxisAlignment = latte.contentAlignment.center,
        size = { latte.size.grow, latte.size.grow },
        style = {
            backgroundColor = latte.color.hex("#f0f0f0"),
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
                        type = "standard",
                        onClick = function()
                            state:setState({ count = state.count + 1 })
                        end
                    })
                }
            }),
            
        }
    })
end)

local window = {
	title = "Counter App",
	size = { 400, 300 },
	children = {
		latte.ui.Counter({})
	}
}
latte.showWindow(window);

latte.runApp()