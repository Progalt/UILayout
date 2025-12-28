
latte.registerComponent("Text", function(props)
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
end)

latte.registerComponent("Container", function(props)
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
end)

latte.registerComponent("BasicButton", function(props)

	local textStr = props[1] or props.text
	local state = latte.useState({
		hovered = false
	})

	return latte.ui.Container({
		padding = latte.padding.axis(16, 6),
		style = latte.mergeStyles({ 
			backgroundColor = state.hovered and latte.color.hex("#f0f4fa") or latte.color.hex("#ffffff"),
			borderRadius = latte.borderRadius.all(4),
			border = {
				width = 1, 
				color = state.hovered and latte.color.hex("#b6c1d2") or latte.color.hex("#d0d5dc")
			}
		}, props.style or {}),
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
				}, props.textStyle or {})
			})
		}

	})
end)

latte.registerComponent("TextField", function(props)

	local state = latte.useState({
		text = props.text or "",
		cursorPosition = #(props.text or "")
	})

	local focusHandle = latte.focus.register()

	local focused = latte.focus.isFocused(focusHandle)

	local placeholderText = props.placeholder or ""

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
		children = {
			-- if text is empty, show placeholder
			((state.text == "" and latte.ui.Text({
				text = placeholderText,
				style = latte.mergeStyles({
					fontSize = 14,
					color = latte.color.hex("#888888")
				}, props.textStyle or {})
			})) or latte.ui.Text({
				text = state.text,
				style = latte.mergeStyles({
					fontSize = 14,
					color = latte.color.hex("#222222")
				}, props.textStyle or {})
			})),
			-- Use an absolute positioned container as a cursor
			-- Only on focused state
			(focused and latte.ui.Container({
				layout = latte.layout.absolute,
				position = {8 + (latte.measureTextWidth(state.text:sub(1, state.cursorPosition), 14)), 7},
				size = {2, 16},
				style = {
					backgroundColor = latte.color.hex("#222222"),
				},
			})) or nil,
		},
		onClick = function()
			latte.focus.request(focusHandle)
			state:setState({})  -- Trigger re-render to show cursor
		end, 
		onKeyDown = function(key)
			local changed = false
			if key == "backspace" then
				if #state.text > 0 then
					-- Remove the last character at cursor position
					state:setState({ 
						text = state.text:sub(1, state.cursorPosition - 1) .. state.text:sub(state.cursorPosition + 1), 
						cursorPosition = state.cursorPosition - 1 
					})
					changed = true
				end
			elseif key == "left" then
				state:setState({ cursorPosition = state.cursorPosition - 1 })
			elseif key == "right" then
				state:setState({ cursorPosition = state.cursorPosition + 1 })
			end

			if changed and props.onTextChange then
				props.onTextChange(state.text)
			end
		end,
		onTextInput = function(inputText)
			-- Insert the input text at the cursor position
			state:setState({ 
				text = state.text:sub(1, state.cursorPosition) .. inputText .. state.text:sub(state.cursorPosition + 1), 
				cursorPosition = state.cursorPosition + #inputText 
			})
			if props.onTextChange then
				props.onTextChange(state.text)
			end
		end
	})
end)