
# TODO

This TODO list covers everything I have identified as needing to be done to make a fairly robust, complete UI library. 
It's probably missing stuff. 

It doesn't feature everything that has been done as it was started when the router was implemented. 
So most basic UI stuff was done at that point. 

### LatteUI

- [X] -> Basic Router
- [X] -> Component Library
- [X] -> Focus API
- [ ] -> DevTools
- [X] -> Losing focus on outside clicks -> Bit rough but works
- [X] -> FontMetrics API
- [ ] -> Better retrieving of Fonts, including System Fonts
- [ ] -> Font Icons
- [ ] -> UI Stack layouts
- [ ] -> JSON API 
- [ ] -> Local Storage API
- [ ] -> SVGs
- [ ] -> Better styling, probably can be built on top of existing styling systems. 
- [ ] -> Styling Themes
- [ ] -> FluentUI Components 
- [ ] -> Material3 Components
- [ ] -> Mouse Events
	- [X] -> Click 
	- [ ] -> Dragging
	- [ ] -> Finish sending the events to the components. 
- [ ] -> Dragging and payloads API 
- [ ] -> Keyboard Events
	- [X] -> Key mods now passed to onKeyDown 
	- [ ] -> onKeyUp
- [ ] -> Clipboard API 
	- [X] -> Text Clipboard
	- [ ] -> Data Clipboard
- [ ] -> Better text editing
	- [X] -> Extracted common text handling logic into useTextEdit 
	- [X] -> Simple Shift selection, with the associated inserting/removing
	- [ ] -> Ctrl+Arrow word jumping
	- [ ] -> Text drag selection 
	- [ ] -> Text Function Hooks
	- [ ] -> Multi-Line text edit
		- [ ] -> Support same basic features as a single line edit
- [ ] -> Cursor API
- [X] -> useEffect API
- [ ] -> Maybe a useMemo like API? 
- [ ] -> URL API -> Maybe use https://github.com/golgote/neturl
- [ ] -> Image widgets
	- [ ] -> Images from files
	- [ ] -> Network Images
- [ ] -> Animation API, to make animated components
- [ ] -> Pass tables to route, or parse args from pattern
- [ ] -> Research into generic API stuff that could be useful
	- [ ] -> ValueNotifier?  
	- [ ] -> Async functions
		- [ ] -> setTimeout 
- [ ] -> Support Multiple Windows -> See NanoVG TODO for multiple VAOs

### Nanovg
- [ ] -> Support a VAO per OpenGL context, currently multiple windows don't work on shared contexts. 
- [ ] -> Research having multiple Gradient stops, probably generate a texture for them over what it is now. 
- [ ] -> Better text rendering.