local function execfunc()
    LogInfo("exec test")
end

local function checkFunc()
    return true
end

Editor.RegisterPlugin("Test", execfunc, checkFunc)
