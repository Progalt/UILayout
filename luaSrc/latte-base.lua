

-- Merges two style tables, with style2 overwriting matching keys in style1
latte.mergeStyles = function(style1, style2)
	local result = {}
    -- Copy all key/value pairs from style1
    for k, v in pairs(style1) do
        result[k] = v
    end
    -- Overwrite/add key/value pairs from style2
    for k, v in pairs(style2) do
        result[k] = v
    end
    return result
end

-- Merges two property tables, excluding certain keys from being overwritten
function latte.mergeProps(base, exclude, extra)
    local result = {}
    for k, v in pairs(base) do result[k] = v end

    local excludeSet = {}
    for _, key in ipairs(exclude) do
        excludeSet[key] = true
    end

    for k, v in pairs(extra) do
        -- Don't overwrite if excluded or already set
        if not excludeSet[k] and result[k] == nil then
            result[k] = v
        end
    end
    return result
end

-- Padding helper functions
-- Padding is just a table of 4 values: { left, top, right, bottom }

latte.padding = {}
latte.padding.all = function(value)
	return { value, value, value, value }
end
latte.padding.axis = function(horizontal, vertical)
	return { horizontal, vertical, horizontal, vertical }
end

-- Padding left, top, right, bottom
-- The padding table is always in this order so this one is just for clarity
latte.padding.ltrb = function(left, top, right, bottom)
	return { left, top, right, bottom }
end

-- Size constants

latte.size = {}
latte.size.fit = -1 -- Special value indicating "fit to content"
latte.size.grow = -2 -- Special value indicating "grow to fill available space"

-- Content alignment constants
-- Used for specifying alignments on the main axis and cross axis

latte.contentAlignment = {}
latte.contentAlignment.atStart = 0	-- Called this to match atEnd
latte.contentAlignment.atEnd = 1 	-- Called atEnd to avoid conflict with Lua's 'end' keyword
latte.contentAlignment.center = 2
latte.contentAlignment.spaceBetween = 3
latte.contentAlignment.spaceAround = 4

-- Layout constants
latte.layout = {}
latte.layout.relative = 0
latte.layout.absolute = 1

-- Backdrop constants
-- These can be passed to the window table backdrop property
latte.backdrop = {}
latte.backdrop.none = 0         -- Just a normal window
latte.backdrop.mica = 1         -- (Windows 11 only) Mica backdrop
latte.backdrop.acrylic = 2      -- On systems that support it, an Acrylic backdrop
latte.backdrop.transparent = 3  -- Transparent backdrop

latte.borderRadius = {}
latte.borderRadius.all = function(value)
	return { value, value, value, value }
end

latte.borderRadius.ltrb = function(left, top, right, bottom)
	return { left, top, right, bottom }
end

-- Latte expects colors in a table, with [1] = r, [2] = g, [3] = b, [4] = a, these are in float format (0.0 - 1.0)
latte.color = {}

-- Create a color from 0 - 255 RGB, A is optional, 0.0 - 1.0 and defaults to 1.0
latte.color.rgba = function(r, g, b, a)
    return { r / 255, g / 255, b / 255, a or 1.0 }
end

-- Takes a hex color string like "#RGB" or "#RGBA" "#RRGGBB" or "#RRGGBBAA" and returns a color table
latte.color.hex = function(hex)
    local r, g, b, a
    if string.sub(hex, 1, 1) == "#" then
        hex = string.sub(hex, 2)
    end
    if #hex == 3 then
        r = tonumber("0x" .. string.sub(hex, 1, 1)) / 15
        g = tonumber("0x" .. string.sub(hex, 2, 2)) / 15
        b = tonumber("0x" .. string.sub(hex, 3, 3)) / 15
        a = 1.0
    elseif #hex == 4 then
        r = tonumber("0x" .. string.sub(hex, 1, 1)) / 15
        g = tonumber("0x" .. string.sub(hex, 2, 2)) / 15
        b = tonumber("0x" .. string.sub(hex, 3, 3)) / 15
        a = tonumber("0x" .. string.sub(hex, 4, 4)) / 15
    elseif #hex == 6 then
        r = tonumber("0x" .. string.sub(hex, 1, 2)) / 255
        g = tonumber("0x" .. string.sub(hex, 3, 4)) / 255
        b = tonumber("0x" .. string.sub(hex, 5, 6)) / 255
        a = 1.0  -- Changed from nil to 1.0 for consistency
    elseif #hex == 8 then
        r = tonumber("0x" .. string.sub(hex, 1, 2)) / 255
        g = tonumber("0x" .. string.sub(hex, 3, 4)) / 255
        b = tonumber("0x" .. string.sub(hex, 5, 6)) / 255
        a = tonumber("0x" .. string.sub(hex, 7, 8)) / 255
    else
        error("Invalid hex color format")
    end

    return { r, g, b, a }
end

-- Create a color table from hsla values
latte.color.hsla = function(h, s, l, a)
    a = a or 1.0
    local r, g, b

    if s == 0 then
        r, g, b = l, l, l -- Achromatic
    else
        local function hue2rgb(p, q, t)
            if t < 0 then t = t + 1 end
            if t > 1 then t = t - 1 end
            if t < 1/6 then return p + (q - p) * 6 * t end
            if t < 1/2 then return q end
            if t < 2/3 then return p + (q - p) * (2/3 - t) * 6 end
            return p
        end

        local q
        if l < 0.5 then
            q = l * (1 + s)
        else
            q = l + s - l * s
        end
        local p = 2 * l - q

        r = hue2rgb(p, q, h + 1/3)
        g = hue2rgb(p, q, h)
        b = hue2rgb(p, q, h - 1/3)
    end

    return { r, g, b, a }   
end

-- CSS Named Colors
latte.color.aliceblue = latte.color.hex("#F0F8FF")
latte.color.antiquewhite = latte.color.hex("#FAEBD7")
latte.color.aqua = latte.color.hex("#00FFFF")
latte.color.aquamarine = latte.color.hex("#7FFFD4")
latte.color.azure = latte.color.hex("#F0FFFF")
latte.color.beige = latte.color.hex("#F5F5DC")
latte.color.bisque = latte.color.hex("#FFE4C4")
latte.color.black = latte.color.hex("#000000")
latte.color.blanchedalmond = latte.color.hex("#FFEBCD")
latte.color.blue = latte.color.hex("#0000FF")
latte.color.blueviolet = latte.color.hex("#8A2BE2")
latte.color.brown = latte.color.hex("#A52A2A")
latte.color.burlywood = latte.color.hex("#DEB887")
latte.color.cadetblue = latte.color.hex("#5F9EA0")
latte.color.chartreuse = latte.color.hex("#7FFF00")
latte.color.chocolate = latte.color.hex("#D2691E")
latte.color.coral = latte.color.hex("#FF7F50")
latte.color.cornflowerblue = latte.color.hex("#6495ED")
latte.color.cornsilk = latte.color.hex("#FFF8DC")
latte.color.crimson = latte.color.hex("#DC143C")
latte.color.cyan = latte.color.hex("#00FFFF")
latte.color.darkblue = latte.color.hex("#00008B")
latte.color.darkcyan = latte.color.hex("#008B8B")
latte.color.darkgoldenrod = latte.color.hex("#B8860B")
latte.color.darkgray = latte.color.hex("#A9A9A9")
latte.color.darkgrey = latte.color.hex("#A9A9A9")
latte.color.darkgreen = latte.color.hex("#006400")
latte.color.darkkhaki = latte.color.hex("#BDB76B")
latte.color.darkmagenta = latte.color.hex("#8B008B")
latte.color.darkolivegreen = latte.color.hex("#556B2F")
latte.color.darkorange = latte.color.hex("#FF8C00")
latte.color.darkorchid = latte.color.hex("#9932CC")
latte.color.darkred = latte.color.hex("#8B0000")
latte.color.darksalmon = latte.color.hex("#E9967A")
latte.color.darkseagreen = latte.color.hex("#8FBC8F")
latte.color.darkslateblue = latte.color.hex("#483D8B")
latte.color.darkslategray = latte.color.hex("#2F4F4F")
latte.color.darkslategrey = latte.color.hex("#2F4F4F")
latte.color.darkturquoise = latte.color.hex("#00CED1")
latte.color.darkviolet = latte.color.hex("#9400D3")
latte.color.deeppink = latte.color.hex("#FF1493")
latte.color.deepskyblue = latte.color.hex("#00BFFF")
latte.color.dimgray = latte.color.hex("#696969")
latte.color.dimgrey = latte.color.hex("#696969")
latte.color.dodgerblue = latte.color.hex("#1E90FF")
latte.color.firebrick = latte.color.hex("#B22222")
latte.color.floralwhite = latte.color.hex("#FFFAF0")
latte.color.forestgreen = latte.color.hex("#228B22")
latte.color.fuchsia = latte.color.hex("#FF00FF")
latte.color.gainsboro = latte.color.hex("#DCDCDC")
latte.color.ghostwhite = latte.color.hex("#F8F8FF")
latte.color.gold = latte.color.hex("#FFD700")
latte.color.goldenrod = latte.color.hex("#DAA520")
latte.color.gray = latte.color.hex("#808080")
latte.color.grey = latte.color.hex("#808080")
latte.color.green = latte.color.hex("#008000")
latte.color.greenyellow = latte.color.hex("#ADFF2F")
latte.color.honeydew = latte.color.hex("#F0FFF0")
latte.color.hotpink = latte.color.hex("#FF69B4")
latte.color.indianred = latte.color.hex("#CD5C5C")
latte.color.indigo = latte.color.hex("#4B0082")
latte.color.ivory = latte.color.hex("#FFFFF0")
latte.color.khaki = latte.color.hex("#F0E68C")
latte.color.lavender = latte.color.hex("#E6E6FA")
latte.color.lavenderblush = latte.color.hex("#FFF0F5")
latte.color.lawngreen = latte.color.hex("#7CFC00")
latte.color.lemonchiffon = latte.color.hex("#FFFACD")
latte.color.lightblue = latte.color.hex("#ADD8E6")
latte.color.lightcoral = latte.color.hex("#F08080")
latte.color.lightcyan = latte.color.hex("#E0FFFF")
latte.color.lightgoldenrodyellow = latte.color.hex("#FAFAD2")
latte.color.lightgray = latte.color.hex("#D3D3D3")
latte.color.lightgrey = latte.color.hex("#D3D3D3")
latte.color.lightgreen = latte.color.hex("#90EE90")
latte.color.lightpink = latte.color.hex("#FFB6C1")
latte.color.lightsalmon = latte.color.hex("#FFA07A")
latte.color.lightseagreen = latte.color.hex("#20B2AA")
latte.color.lightskyblue = latte.color.hex("#87CEFA")
latte.color.lightslategray = latte.color.hex("#778899")
latte.color.lightslategrey = latte.color.hex("#778899")
latte.color.lightsteelblue = latte.color.hex("#B0C4DE")
latte.color.lightyellow = latte.color.hex("#FFFFE0")
latte.color.lime = latte.color.hex("#00FF00")
latte.color.limegreen = latte.color.hex("#32CD32")
latte.color.linen = latte.color.hex("#FAF0E6")
latte.color.magenta = latte.color.hex("#FF00FF")
latte.color.maroon = latte.color.hex("#800000")
latte.color.mediumaquamarine = latte.color.hex("#66CDAA")
latte.color.mediumblue = latte.color.hex("#0000CD")
latte.color.mediumorchid = latte.color.hex("#BA55D3")
latte.color.mediumpurple = latte.color.hex("#9370DB")
latte.color.mediumseagreen = latte.color.hex("#3CB371")
latte.color.mediumslateblue = latte.color.hex("#7B68EE")
latte.color.mediumspringgreen = latte.color.hex("#00FA9A")
latte.color.mediumturquoise = latte.color.hex("#48D1CC")
latte.color.mediumvioletred = latte.color.hex("#C71585")
latte.color.midnightblue = latte.color.hex("#191970")
latte.color.mintcream = latte.color.hex("#F5FFFA")
latte.color.mistyrose = latte.color.hex("#FFE4E1")
latte.color.moccasin = latte.color.hex("#FFE4B5")
latte.color.navajowhite = latte.color.hex("#FFDEAD")
latte.color.navy = latte.color.hex("#000080")
latte.color.oldlace = latte.color.hex("#FDF5E6")
latte.color.olive = latte.color.hex("#808000")
latte.color.olivedrab = latte.color.hex("#6B8E23")
latte.color.orange = latte.color.hex("#FFA500")
latte.color.orangered = latte.color.hex("#FF4500")
latte.color.orchid = latte.color.hex("#DA70D6")
latte.color.palegoldenrod = latte.color.hex("#EEE8AA")
latte.color.palegreen = latte.color.hex("#98FB98")
latte.color.paleturquoise = latte.color.hex("#AFEEEE")
latte.color.palevioletred = latte.color.hex("#DB7093")
latte.color.papayawhip = latte.color.hex("#FFEFD5")
latte.color.peachpuff = latte.color.hex("#FFDAB9")
latte.color.peru = latte.color.hex("#CD853F")
latte.color.pink = latte.color.hex("#FFC0CB")
latte.color.plum = latte.color.hex("#DDA0DD")
latte.color.powderblue = latte.color.hex("#B0E0E6")
latte.color.purple = latte.color.hex("#800080")
latte.color.red = latte.color.hex("#FF0000")
latte.color.rosybrown = latte.color.hex("#BC8F8F")
latte.color.royalblue = latte.color.hex("#4169E1")
latte.color.saddlebrown = latte.color.hex("#8B4513")
latte.color.salmon = latte.color.hex("#FA8072")
latte.color.sandybrown = latte.color.hex("#F4A460")
latte.color.seagreen = latte.color.hex("#2E8B57")
latte.color.seashell = latte.color.hex("#FFF5EE")
latte.color.sienna = latte.color.hex("#A0522D")
latte.color.silver = latte.color.hex("#C0C0C0")
latte.color.skyblue = latte.color.hex("#87CEEB")
latte.color.slateblue = latte.color.hex("#6A5ACD")
latte.color.slategray = latte.color.hex("#708090")
latte.color.slategrey = latte.color.hex("#708090")
latte.color.snow = latte.color.hex("#FFFAFA")
latte.color.springgreen = latte.color.hex("#00FF7F")
latte.color.steelblue = latte.color.hex("#4682B4")
latte.color.tan = latte.color.hex("#D2B48C")
latte.color.teal = latte.color.hex("#008080")
latte.color.thistle = latte.color.hex("#D8BFD8")
latte.color.tomato = latte.color.hex("#FF6347")
latte.color.turquoise = latte.color.hex("#40E0D0")
latte.color.violet = latte.color.hex("#EE82EE")
latte.color.wheat = latte.color.hex("#F5DEB3")
latte.color.white = latte.color.hex("#FFFFFF")
latte.color.whitesmoke = latte.color.hex("#F5F5F5")
latte.color.yellow = latte.color.hex("#FFFF00")
latte.color.yellowgreen = latte.color.hex("#9ACD32")