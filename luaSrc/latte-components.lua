
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