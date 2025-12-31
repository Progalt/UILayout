
# TODO

### LatteUI

- [X] -> Basic Router
- [X] -> Component Library
- [X] -> Focus API
- [ ] -> DevTools
- [X] -> Losing focus on outside clicks -> Bit rough but works
- [X] -> FontMetrics API
- [ ] -> Better retrieving of Fonts, including System Fonts
- [ ] -> Font Icons
- [ ] -> SVGs
- [ ] -> Mouse Events
	- [X] -> Click 
	- [ ] -> Dragging
	- [ ] -> Finish sending the events to the components. 
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
- [ ] -> Cursor API
- [ ] -> useEffect API
- [ ] -> URL API -> Maybe use https://github.com/golgote/neturl
- [ ] -> Image widgets
	- [ ] -> Images from files
	- [ ] -> Network Images
- [ ] -> Animation API, to make animated components
- [ ] -> Pass tables to route, or parse args from pattern
- [ ] -> Look into making Async stuff 
- [ ] -> Support Multiple Windows -> See NanoVG TODO for multiple VAOs

### Nanovg
- [ ] -> Support a VAO per OpenGL context, currently multiple windows don't work on shared contexts. 
- [ ] -> Research having multiple Gradient stops, probably generate a texture for them over what it is now. 
- [ ] -> Better text rendering.