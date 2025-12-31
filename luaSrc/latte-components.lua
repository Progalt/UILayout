

local base = latte.createComponentLibrary("ui")

local function Text(props)
	-- Text can be specified as the first array element in the table or as the 'text' property
    local textStr = props[1] or props.text

    return latte.mergeProps(
        {
            text = textStr or "",
            style = latte.mergeStyles({
				fontSize = 14,
				color = latte.color.hex("#000000ff"),
			}, props.style or {}),
        },
        {1, "text", "style", "fontSize"},
        props
    )
end

local function Container(props)
	return latte.mergeProps({
		mainAxisAlignment = props.mainAxisAlignment or latte.contentAlignment.atStart,
		crossAxisAlignment = props.crossAxisAlignment or latte.contentAlignment.atStart,
		padding = props.padding or { 0, 0, 0, 0},
		size = props.size or { latte.size.fit, latte.size.fit },
		spacing = props.spacing or 0,
		children = props.children or {},
		style = latte.mergeStyles({}, props.style or {})
	},
	{ "padding", "size", "spacing", "children", "style" },
		props
	)
end

local function BasicButton(props)
	local textStr = props[1] or props.text
	local state = latte.useState({
		hovered = false
	})

	local style = props.style or {}
	if state.hovered and props.hoveredStyle then
		style = latte.mergeStyles(style, props.hoveredStyle)
	end

	local textStyle = props.textStyle or {}
	if state.hovered and props.textHoveredStyle then
		textStyle = latte.mergeStyles(textStyle, props.textHoveredStyle)
	end

	return latte.ui.Container({
		padding = latte.padding.axis(16, 6),
		style = latte.mergeStyles({ 
			backgroundColor = state.hovered and latte.color.hex("#f0f4fa") or latte.color.hex("#ffffff"),
			borderRadius = latte.borderRadius.all(4),
			border = {
				width = 1, 
				color = state.hovered and latte.color.hex("#b6c1d2") or latte.color.hex("#d0d5dc")
			}
		}, style),
		mainAxisAlignment = latte.contentAlignment.center, 
		crossAxisAlignment = latte.contentAlignment.center,
		onClick = props.onClick or function() end,
		onHoverEnter = function()
			state:setState({ hovered = true })
		end,
		onHoverExit = function()
			state:setState({ hovered = false })
		end,
		children = {
			latte.ui.Text({
				text = textStr,
				style = latte.mergeStyles({
					fontSize = 14,
					color = latte.color.hex("#222222")
				}, textStyle)
			})
		}

	})
end

local function VBox(props)
	return {
		direction = "vertical",
		mainAxisAlignment = props.mainAxisAlignment or latte.contentAlignment.atStart,
		crossAxisAlignment = props.crossAxisAlignment or latte.contentAlignment.atStart,
		padding = props.padding or { 0, 0, 0, 0},
		spacing = props.spacing or 0,
		size = { latte.size.grow, latte.size.grow },
		children = props.children or {},
	}
end

local function HBox(props)
	return {
		direction = "horizontal",
		mainAxisAlignment = props.mainAxisAlignment or latte.contentAlignment.atStart,
		crossAxisAlignment = props.crossAxisAlignment or latte.contentAlignment.atStart,
		padding = props.padding or { 0, 0, 0, 0},
		spacing = props.spacing or 0,
		size = { latte.size.grow, latte.size.grow },
		children = props.children or {},
	}
end

local function TextField(props)
	
	local edit = latte.useTextEdit(props.text or "")

	local focusHandle = latte.useFocus()

	local focused = focusHandle:isFocused()

	local placeholderText = props.placeholder or ""

	local fontMetrics = latte.getFontMetrics("Roboto-Regular", 14)

	local selection = edit.state.selection
	if selection and selection.start ~= selection.stop then
		-- Always sort for left/right
		local selStart = math.min(selection.start, selection.stop)
		local selStop = math.max(selection.start, selection.stop)
		local left = 8 + fontMetrics:getTextSize(edit.state.text:sub(1, selStart)).width
		local selectionText = edit.state.text:sub(selStart + 1, selStop)
		local selWidth = fontMetrics:getTextSize(selectionText).width
	end

	return latte.ui.Container({
		padding = latte.padding.axis(8, 4),
		style = latte.mergeStyles({
			backgroundColor = latte.color.hex("#ffffffff"),
			borderRadius = latte.borderRadius.all(4),
			border = {
				width = 1,
				color = latte.color.hex("#d0d5dc")
			}
		}, props.style or {}),
		size = props.size or { latte.size.grow, 30 },
		mainAxisAlignment = latte.contentAlignment.start,
		crossAxisAlignment = latte.contentAlignment.center,
		children = (function()
			local result = {}
			if selection and selection.start ~= selection.stop then
				local selStart = math.min(selection.start, selection.stop)
				local selStop = math.max(selection.start, selection.stop)
				local left = 8 + fontMetrics:getTextSize(edit.state.text:sub(1, selStart)).width
				local selectionText = edit.state.text:sub(selStart + 1, selStop)
				local selWidth = fontMetrics:getTextSize(selectionText).width
				
				table.insert(result, latte.ui.Container({
					layout = latte.layout.absolute,
					position = { left, 7 },
					size = { selWidth, 16 },
					style = {
						backgroundColor = latte.color.hex("#cccccc"),
					},
				}))
			end

			-- Show placeholder or text
			if edit.state.text == "" then
				table.insert(result, latte.ui.Text({
					text = placeholderText,
					style = latte.mergeStyles({
						fontSize = 14,
						color = latte.color.hex("#888888")
					}, props.textStyle or {})
				}))
			else
				table.insert(result, latte.ui.Text({
					text = edit.state.text,
					style = latte.mergeStyles({
						fontSize = 14,
						color = latte.color.hex("#222222")
					}, props.textStyle or {})
				}))
			end

			-- Cursor
			if focused then
				table.insert(result, latte.ui.Container({
					layout = latte.layout.absolute,
					position = {8 + (fontMetrics:getTextSize(edit.state.text:sub(1, edit.state.cursor)).width), 7},
					size = {1, 16},
					style = {
						backgroundColor = latte.color.hex("#222222"),
					},
				}))
			end

			return result
		end)(),
		onClick = function()
			focusHandle:request()
			edit.state:setState({})  -- Trigger re-render to show cursor
		end, 
		onKeyDown = function(key, keyMods)
			
			if key == "backspace" then
				edit.removeLeft()
			elseif key == "left" then
				edit.moveCursor(-1, keyMods.leftShift)
			elseif key == "right" then
				edit.moveCursor(1, keyMods.leftShift)
			end
		end,
		onTextInput = function(str)
			edit.insert(str)
			if props.onTextChange then
				props.onTextChange(edit.state.text)
			end
		end,
	})
end

-- Register all components
base:registerAll {
	["Text"] = Text,
	["Container"] = Container,
	["BasicButton"] = BasicButton,
	["VBox"] = VBox,
	["HBox"] = HBox,
	["TextField"] = TextField,
}
