local function execfunc()
    local vertices = {}
    table.insert(vertices, Vec2.new(-352, 0))
    table.insert(vertices, Vec2.new(224, -32))
    Editor.InsertLines(vertices, false)
    vertices = {}
    table.insert(vertices, Vec2.new(-192, 224))
    table.insert(vertices, Vec2.new(-72.086121, -15.550771))
    Editor.InsertLines(vertices, false)
end

Editor.RegisterPlugin("SlopeTest", execfunc)
   