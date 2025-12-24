
-- Simple Hello World using LatteUI

local mainWindow = {
	title = "My Window",
	size = { 300, 200 },
	padding = latte.padding.all(10),
	children = { 
		latte.ui.Container({
			padding = latte.padding.all(16),
			size = { latte.size.grow, latte.size.grow },
			mainAxisAlignment = latte.contentAlignment.center, 
			crossAxisAlignment = latte.contentAlignment.center,
			style = {
				backgroundColor = latte.color.lavenderblush,
			},
			children = {
				latte.ui.Container({ 
						size = { latte.size.grow, latte.size.grow }, 
						style = { 
							backgroundColor = latte.color.cornflowerblue,
							borderRadius = latte.borderRadius.all(12),
						},
						mainAxisAlignment = latte.contentAlignment.center, 
						crossAxisAlignment = latte.contentAlignment.center,
						children = {
							latte.ui.Text({ 
								"Hello World", 
								style = {
									fontSize = 24,
								}
							})
						}
					}),
			}
		})
	}
}

latte.showWindow(mainWindow)

latte.runApp()