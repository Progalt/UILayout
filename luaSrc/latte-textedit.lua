

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

local function useMultiLineTextEdit(initText, maxLines)
    local function splitByNewLine(text) 
        local lines = {}
        if not text or text == "" then
            return { "" }
        end
        for line in (text .. "\n"):gmatch("(.-)\n") do
            table.insert(lines, line)
        end
        return lines
    end

    local state = latte.useState({
        lines = splitByNewLine(initText),
        cursor = { line = 1, offset = 0 },
        selection = { 
            start = { line = 1, offset = 0 },
            stop = { line = 1, offset = 0 }
        }
    })

    local function collapsedSelection(pos)
        return { 
            start = { line = pos.line, offset = pos.offset }, 
            stop = { line = pos.line, offset = pos.offset } 
        }
    end

    local function positionsEqual(a, b)
        return a.line == b.line and a.offset == b.offset
    end

    local function positionLessThan(a, b)
        return a.line < b.line or (a.line == b.line and a.offset < b.offset)
    end

    local function getOrderedSelection(sel)
        local start, stop = sel.start, sel.stop
        if positionLessThan(start, stop) or positionsEqual(start, stop) then
            return start, stop
        else
            return stop, start
        end
    end

    local function hasSelection(sel)
        return not positionsEqual(sel.start, sel.stop)
    end

    local function deleteSelection(lines, selStart, selStop)
        if selStart.line == selStop.line then
            -- Single line selection
            local line = lines[selStart.line]
            lines[selStart.line] = line:sub(1, selStart.offset) .. line:sub(selStop.offset + 1)
        else
            -- Multi-line selection - rebuild the lines array properly
            local newLines = {}
            
            -- Copy lines before selection
            for i = 1, selStart.line - 1 do
                newLines[i] = lines[i]
            end
            
            -- Merge the partial lines at selection boundaries
            local firstPart = lines[selStart.line]:sub(1, selStart.offset)
            local lastPart = lines[selStop.line]:sub(selStop.offset + 1)
            newLines[selStart.line] = firstPart .. lastPart
            
            -- Copy lines after selection, adjusting indices
            local newIndex = selStart.line + 1
            for i = selStop.line + 1, #lines do
                newLines[newIndex] = lines[i]
                newIndex = newIndex + 1
            end
            
            -- Replace the lines array contents
            for i = #lines, 1, -1 do
                lines[i] = nil
            end
            for i = 1, #newLines do
                lines[i] = newLines[i]
            end
        end
        return selStart
    end

    local function insert(text)
        local lines = state.lines -- Work with existing array
        local cursor = state.cursor
        local sel = state.selection

        -- If there's a selection, delete it first
        if hasSelection(sel) then
            local selStart, selStop = getOrderedSelection(sel)
            cursor = deleteSelection(lines, selStart, selStop)
        end

        -- Insert new text at cursor
        local newLines = splitByNewLine(text)
        local curLineIdx = cursor.line
        local curOffset = cursor.offset
        local curLine = lines[curLineIdx] or ""

        if #newLines == 1 then
            -- Single line insertion
            local before = curLine:sub(1, curOffset)
            local after = curLine:sub(curOffset + 1)
            lines[curLineIdx] = before .. newLines[1] .. after
            cursor = { line = curLineIdx, offset = curOffset + #newLines[1] }
        else
            -- Multi-line insertion - rebuild properly
            local resultLines = {}
            
            -- Copy lines before cursor
            for i = 1, curLineIdx - 1 do
                resultLines[i] = lines[i]
            end
            
            -- Handle the line being split
            local before = curLine:sub(1, curOffset)
            local after = curLine:sub(curOffset + 1)
            
            -- Insert first new line with before text
            resultLines[curLineIdx] = before .. newLines[1]
            
            -- Insert middle lines
            for i = 2, #newLines - 1 do
                resultLines[curLineIdx + i - 1] = newLines[i]
            end
            
            -- Insert last new line with after text
            local lastNewLineIdx = curLineIdx + #newLines - 1
            resultLines[lastNewLineIdx] = newLines[#newLines] .. after
            
            -- Copy remaining lines after cursor
            local nextIdx = lastNewLineIdx + 1
            for i = curLineIdx + 1, #lines do
                resultLines[nextIdx] = lines[i]
                nextIdx = nextIdx + 1
            end
            
            -- Replace lines array contents
            for i = #lines, 1, -1 do
                lines[i] = nil
            end
            for i = 1, #resultLines do
                lines[i] = resultLines[i]
            end
            
            cursor = {
                line = lastNewLineIdx,
                offset = #newLines[#newLines]
            }
        end

        state:setState({
            lines = lines,
            cursor = cursor,
            selection = collapsedSelection(cursor)
        })
    end

    local function moveCursor(delta, shouldSelect)
        local lines = state.lines
        local numLines = #lines
        local oldCursor = state.cursor
        local sel = state.selection
        
        local newLine = math.max(1, math.min(oldCursor.line + (delta[2] or 0), numLines))
        local lineText = lines[newLine] or ""
        local lineLen = #lineText

        local newOffset
        if delta[2] and delta[2] ~= 0 then
            -- Vertical movement: try to preserve horizontal position
            newOffset = math.min(oldCursor.offset, lineLen)
        else
            -- Horizontal movement
            newOffset = oldCursor.offset + (delta[1] or 0)
            
            if newOffset < 0 and delta[1] < 0 and newLine > 1 then
                -- Move to end of previous line
                newLine = newLine - 1
                newOffset = #(lines[newLine] or "")
            elseif newOffset > lineLen and delta[1] > 0 and newLine < numLines then
                -- Move to start of next line
                newLine = newLine + 1
                newOffset = 0
            else 
                newOffset = math.max(0, math.min(newOffset, lineLen))
            end
           
        end

        local newPos = { line = newLine, offset = newOffset }

        local newSelection
        if shouldSelect then
            if hasSelection(sel) then
                -- Extend existing selection - keep original start
                newSelection = { start = sel.start, stop = newPos }
            else
                -- Start new selection from old cursor position
                newSelection = { start = oldCursor, stop = newPos }
            end
        else
            -- Not selecting - handle existing selection properly
            if hasSelection(sel) then
                local selStart, selStop = getOrderedSelection(sel)
                -- Move to appropriate edge based on direction
                if (delta[1] and delta[1] < 0) or (delta[2] and delta[2] < 0) then
                    newPos = selStart
                elseif (delta[1] and delta[1] > 0) or (delta[2] and delta[2] > 0) then
                    newPos = selStop
                end
                -- If no movement delta, just collapse at current cursor
            end
            newSelection = collapsedSelection(newPos)
        end

        state:setState({
            lines = lines,
            cursor = newPos,
            selection = newSelection
        })
    end

    local function removeLeft()
        local lines = state.lines
        local cursor = state.cursor
        local sel = state.selection

        if hasSelection(sel) then
            -- Remove selection
            local selStart, selStop = getOrderedSelection(sel)
            local newCursor = deleteSelection(lines, selStart, selStop)
            
            state:setState({
                lines = lines,
                cursor = newCursor,
                selection = collapsedSelection(newCursor)
            })
        else
            -- Backspace behavior
            if cursor.offset > 0 then
                -- Remove character to the left
                local lineText = lines[cursor.line]
                lines[cursor.line] = lineText:sub(1, cursor.offset - 1) .. lineText:sub(cursor.offset + 1)
                local newCursor = { line = cursor.line, offset = cursor.offset - 1 }
                
                state:setState({
                    lines = lines,
                    cursor = newCursor,
                    selection = collapsedSelection(newCursor)
                })
            elseif cursor.line > 1 then
                -- Merge with previous line
                local prevLine = lines[cursor.line - 1]
                local curLine = lines[cursor.line]
                lines[cursor.line - 1] = prevLine .. curLine
                table.remove(lines, cursor.line)
                
                local newCursor = { line = cursor.line - 1, offset = #prevLine }
                
                state:setState({
                    lines = lines,
                    cursor = newCursor,
                    selection = collapsedSelection(newCursor)
                })
            end
        end
    end

    local function getOrderedSelectionLinesOffsets()
        local start, stop = getOrderedSelection(state.selection)
        return start.line, start.offset, stop.line, stop.offset
    end

    return {
        state = state,
        moveCursor = moveCursor,
        insert = insert,
        removeLeft = removeLeft,
        getOrderedSelectionLinesOffsets = getOrderedSelectionLinesOffsets,
    }
end

latte.useTextEdit = useTextEdit
latte.useMultiLineTextEdit = useMultiLineTextEdit