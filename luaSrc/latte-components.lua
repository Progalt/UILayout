
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