

latte.registerComponent("Container", function(props)

	print("Rebuilding Container: " .. latte.getID())

	return {
		padding = props.padding or { 0, 0, 0, 0},
		size = props.size or { -1, -1 },
		spacing = props.spacing or 0,
		children = props.children or {},
		onPaint = function()
			return false
		end
	}
end)


latte.registerComponent("MyComponent", function(props)
	local state = latte.useState({})

	print("Rebuilding MyComponent: " .. latte.getID())

	return {
		size = { 20, 100 }, 
		style = {
			backgroundColor = "#ffa07a",
			borderRadius = { 8, 8, 8, 8 }
		},
		children = props.children or {}
	}
end)

local mainWindow = {
	title = "My Window",
	size = { 300, 200 },
	padding = { 8, 16, 16, 16 },
	children = { 
		latte.ui.Container({
			padding = { 4, 4, 4, 4 },
			spacing = 6,
			children = {
				latte.ui.Container({ size = { 10, 10 }}),
				latte.ui.MyComponent({}),
				latte.ui.Container({ size = { 10, 10 }})
			}
		})
	}
}

latte.showWindow(mainWindow)

latte.runApp()