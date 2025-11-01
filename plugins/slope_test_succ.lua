local function execfunc()
    local vertices = {}
    table.insert(vertices, Vec2.new(-288, 0))
    table.insert(vertices, Vec2.new(192, -32))
    Editor.InsertLines(vertices, false)
    vertices = {}
    table.insert(vertices, Vec2.new(-160, 224))
    table.insert(vertices, Vec2.new(-30.171265, -17.188581))
    Editor.InsertLines(vertices, false)
end

Editor.RegisterPlugin("SlopeTestSuccess", execfunc)
   