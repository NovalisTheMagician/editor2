local function execfunc()
    local selection = Editor.GetSelection()
    print(selection.mode, #selection.items, selection.items[1].a)
end

local function checkFunc()
    return Editor.CheckSelection("line", 1)
end

Editor.RegisterPlugin("Test", execfunc, checkFunc)