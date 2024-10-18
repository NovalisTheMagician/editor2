local function execfunc()
    local range = 2048
    local vertices = {}
    for i=1,100 do
        local x = math.random(-range, range)
        local y = math.random(-range, range)
        table.insert(vertices, Vec2.new(x, y))
    end
    Editor.InsertLines(vertices, true)
end

Editor.RegisterPlugin("Create Random Lines", execfunc)
