local function execfunc()
    local tbl = { name = "HelloTable" }
    tbl = setmetatable(tbl, {
        __tostring = function (tbl)
            return tbl.name
        end
    })
    print(tbl)
    print(math.sin(math.pi/2))

    a = Vec2.new(1, 2)
    b = Vec2.new(3, 4)
    c = Vec2.new(5, 6)
    d = (a + b) * 5 - c
    print(a, b, c, d)
end

local function checkFunc()
    return true
end

Editor.RegisterPlugin("Test", execfunc, checkFunc)
