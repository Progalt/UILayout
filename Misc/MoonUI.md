# MoonUI 

MoonUI is a UI library, with UI descriptions written in Lua, and logic able 
to be written in Lua or C. 



```lua
-- Widgets are just reusable functions
function MyButton(props)
	return Button {
		style = {
			backgroundColor = "#FF0000",
			color = "#FFFFFF",
			borderRadius = BorderRadius.all(8)
		}

		onClick = function() print("Clicked") end,

		children = {
			Text{ text = props.text }
		}
	}
end

myWindow = {
	
	width = Sizer.grow(),
	height =  Sizer.grow(),

	title = "Hello MoonUI",

	children = {
		VBox{
			children = {
				MyButton{ text = "Button1" },
				MyButton{ text = "Button2" }
			}
		}
	}
}

-- Show the window
moonui.showWindow(myWindow)

-- Run the app
moonui.app.execute()

```