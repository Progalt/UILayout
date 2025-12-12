
# UI Descriptor Landuage Idea

This is an idea I had for a UI descriptor language similar in concept to QML or HTML. 
I wanted something simple with simple syntax. That was easily interfacable with another language like C. 
I like building functionality in the C/C++ but not descripting the UI in those languages. 

My idea is to have an API as simple as
```c
UIElement* elem = getElementById("blah"); 

attachEventHandler(elem, "click", clickHandler);
```
Or something like that. 

## 

```
Panel {
	# This is a comment

	id: "panel1"
	width: Sizer.grow
	height: Sizer.fixed(124)

	# Children are created by using a valid
	# Element name and then a { } 
	Button {
		id: "button1"
	}
	
	Button {
		
	}
}

```

By prefixing a widget with define you can make a new widget that can be reused. 
These can use the base shape/text stuff like below or be a collection of common
widgets.

```
define MyButton {
	Rectangle {
		
		color: Colors.red
		borderRadius: BorderRadius.all(16)

		Text {
			text: "MyButton"
		}
	}
}

```

```
events += [
	Event.click
	Event.hoverEnter
	Event.hoverExit
]
```

```
events -= [
	Event.click
]
```