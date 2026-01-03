

local material = latte.createComponentLibrary("material")

local function Button(props)
    local t = props[1] or props.text

    return latte.ui.BasicButton({
        text = t or "Button",
        onClick = props.onClick,
        size = { latte.size.fit, 40 },
        style = {
            backgroundColor = latte.color.hex("#6750A4"),
            borderRadius = latte.borderRadius.all(100),
            spacing = 8, 
            padding = latte.padding.axis(24, 12),
        },
        hoveredStyle = {
            backgroundColor = latte.color.hex("#836cc0ff"),
            borderRadius = latte.borderRadius.all(100),
            spacing = 8, 
            padding = latte.padding.axis(24, 12),
        },
        textStyle = {
            color = latte.color.hex("#ffffff"),
            fontSize = 14,
        }
    })
end

material:registerAll{
    ["Button"] = Button
}