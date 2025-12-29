-- These components are styled to match Microsoft Fluent UI design principles.
-- Reference: https://developer.microsoft.com/en-us/fluentui#/

fluentui = latte.createComponentLibrary("fluent")

fluentui:register("Button", function(props)

    local type = props.type or "accent"

    local bg = latte.color.hex("#005fb8")
    local hoverBg = latte.color.hex("#196ebf")
    local hoverBorder = latte.color.hex("#2b79c4")
    local borderCol = latte.color.hex("#146cbe")
    local t = props[1] or props.text
    local textCol = latte.color.hex("#ffffff")

    if type ~= "accent" then
        bg = latte.color.hex("#fdfdfd")
        borderCol = latte.color.hex("#ebebeb")
        textCol = latte.color.hex("#1b1b1b")
    end

    return latte.ui.BasicButton({
        text = t or "Button",
        onClick = props.onClick,
        style = {
            backgroundColor = bg,
            borderRadius = latte.borderRadius.all(4),
            spacing = 6, 
            padding = latte.padding.axis(16, 8),
            border = {
                width = 1,
                color = borderCol,
            }
        },
        hoveredStyle = {
            backgroundColor = hoverBg,
            borderRadius = latte.borderRadius.all(4),
            spacing = 6, 
            padding = latte.padding.axis(16, 8),
            border = {
                width = 1,
                color = hoverBorder,
            }
        },
        textStyle = {
            color = textCol,
            fontSize = 14,
        }
    })
end)