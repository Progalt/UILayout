local mainWindow = {
	title = "BasicButton",
	size = { 300, 200 },
	padding = latte.padding.all(32),
	children = { 
		latte.ui.Container({
			id = "BaseContainer",
			size = { latte.size.grow, latte.size.grow },
			mainAxisAlignment = latte.contentAlignment.center, 
			crossAxisAlignment = latte.contentAlignment.center,
			style = {
				backgroundColor = latte.color.lavenderblush,
			},
			children = {
                -- latte.ui.BasicButton({
                --     "Click Me!",
                --     onClick = function()
                --         print("BasicButton Clicked!")
                --     end
                -- })

				latte.ui.TextField({
					placeholder = "Type here...",
				})
            }
		})
	}
}

latte.showWindow(mainWindow)

latte.runApp()