local function useTextEdit(initText)
    -- Always keep selection; collapsed if start==stop, "none" if both are -1
    local function collapsedSelection(index)
        return { start = index, stop = index }
    end

    local state = latte.useState({
        text = initText or "",
        cursor = #(initText or ""), -- cursor is an integer index: 0 = before first char
        selection = collapsedSelection(#(initText or "")),
    })

    -- Returns true if there is a real range selected
    local function hasSelection()
        local sel = state.selection
        return (sel and sel.start ~= sel.stop)
            and (sel.start >= 0 and sel.stop >= 0)
    end

    -- Returns selStart, selStop always in left/right order (may be equal if collapsed)
    local function getOrderedSelection()
        local sel = state.selection
        local s, e = sel.start, sel.stop
        return math.min(s, e), math.max(s, e)
    end

    -- Set text and move cursor, always collapse selection to cursor
    local function setText(newText, cursor)
        cursor = cursor or #newText
        state:setState({
            text = newText,
            cursor = cursor,
            selection = collapsedSelection(cursor),
        })
    end

    -- Insert text at cursor or replace current selection
    local function insert(str)
        local text = state.text
        local cursor = state.cursor
        if hasSelection() then
            local selStart, selStop = getOrderedSelection()
            local newText = text:sub(1, selStart) .. str .. text:sub(selStop + 1)
            local newCursor = selStart + #str
            state:setState({
                text = newText,
                cursor = newCursor,
                selection = collapsedSelection(newCursor),
            })
        else
            local newText = text:sub(1, cursor) .. str .. text:sub(cursor + 1)
            local newCursor = cursor + #str
            state:setState({
                text = newText,
                cursor = newCursor,
                selection = collapsedSelection(newCursor),
            })
        end
    end

    -- Backspace (left-delete) or delete selection
    local function removeLeft()
        local text = state.text
        local cursor = state.cursor
        if hasSelection() then
            local selStart, selStop = getOrderedSelection()
            local newText = text:sub(1, selStart) .. text:sub(selStop + 1)
            state:setState({
                text = newText,
                cursor = selStart,
                selection = collapsedSelection(selStart),
            })
        elseif cursor > 0 then
            local newText = text:sub(1, cursor - 1) .. text:sub(cursor + 1)
            local newCursor = cursor - 1
            state:setState({
                text = newText,
                cursor = newCursor,
                selection = collapsedSelection(newCursor),
            })
        end
    end

    -- Forward (right) delete or delete selection
    local function removeRight()
        local text = state.text
        local cursor = state.cursor
        if hasSelection() then
            local selStart, selStop = getOrderedSelection()
            local newText = text:sub(1, selStart) .. text:sub(selStop + 1)
            state:setState({
                text = newText,
                cursor = selStart,
                selection = collapsedSelection(selStart),
            })
        elseif cursor < #text then
            local newText = text:sub(1, cursor) .. text:sub(cursor + 2)
            state:setState({
                text = newText,
                cursor = cursor,
                selection = collapsedSelection(cursor),
            })
        end
    end

    -- Move cursor left/right and optionally extend selection (for shift+arrows)
    local function moveCursor(delta, shouldSelect)
        local text = state.text
        local len = #text
        local oldCursor = state.cursor
        local sel = state.selection or collapsedSelection(oldCursor)
        if shouldSelect then
            -- Standard shift + left/right selection logic.
            local selection = { start = sel.start, stop = math.max(0, math.min(len, oldCursor + delta)) }
            if sel.start == sel.stop then selection.start = oldCursor end
            state:setState({
                text = text,
                cursor = selection.stop,
                selection = selection,
            })
        else
            if hasSelection() then
                -- Move cursor to the relevant edge of selection based on direction.
                local selStart, selStop = getOrderedSelection()
                local newCursor
                if delta < 0 then
                    newCursor = selStart
                else
                    newCursor = selStop
                end
                state:setState({
                    text = text,
                    cursor = newCursor,
                    selection = collapsedSelection(newCursor),
                })
            else
                -- No selection, just move normally.
                local newCursor = math.max(0, math.min(len, oldCursor + delta))
                state:setState({
                    text = text,
                    cursor = newCursor,
                    selection = collapsedSelection(newCursor),
                })
            end
        end
    end

    -- Set any selection; set both start/stop to the same for collapsed/no selection
    local function setSelection(start, stop)
        state:setState({
            text = state.text,
            cursor = stop,
            selection = { start = start, stop = stop },
        })
    end

    return {
        state = state,
        setText = setText,
        insert = insert,
        removeLeft = removeLeft,
        removeRight = removeRight,
        moveCursor = moveCursor,
        setSelection = setSelection,
        hasSelection = hasSelection,
        getOrderedSelection = getOrderedSelection,
    }
end

latte.useTextEdit = useTextEdit