
-- Simple Hello World using LatteUI

local windowUI = {
	latte.ui.Container({ 
		size = { latte.size.grow, latte.size.grow }, 
		style = { 
			backgroundColor = latte.color.cornflowerblue,
			borderRadius = latte.borderRadius.all(12),
			border = {
				width = 4, 
				color = latte.color.blue
			}
		},
		mainAxisAlignment = latte.contentAlignment.center, 
		crossAxisAlignment = latte.contentAlignment.center,
		children = {
			latte.ui.Text({ 
				"Hello World", 
				id = "HelloText",
				style = {
					color = latte.color.lavenderblush,
					fontSize = 24,
				}
			}),
		}
	}),
}

local mainWindow = {
	title = "My Window",
	size = { 300, 200 },
	padding = latte.padding.all(10),
	children = { 
		latte.ui.Container({
			id = "BaseContainer",
			padding = latte.padding.all(16),
			size = { latte.size.grow, latte.size.grow },
			mainAxisAlignment = latte.contentAlignment.center, 
			crossAxisAlignment = latte.contentAlignment.center,
			style = {
				-- backgroundColor = latte.color.lavenderblush,
			},
			children = windowUI
		})
	}
}

latte.showWindow(mainWindow)

latte.runApp()