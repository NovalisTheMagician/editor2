local function execfunc()
    local tbl = { name = "HelloTable" }
    tbl = setmetatable(tbl, {
        __tostring = function (tbl)
            return tbl.name
        end
    })
    print(tbl)
    print(math.sin(math.pi/2))
end

local function checkFunc()
    return true
end

Editor.RegisterPlugin("Test", execfunc, checkFunc)
