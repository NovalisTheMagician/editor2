local function execfunc()
    local vertices = {}
    table.insert(vertices, Vec2.new(128, 128))
    table.insert(vertices, Vec2.new(-128, 128))
    table.insert(vertices, Vec2.new(-128, -128))
    table.insert(vertices, Vec2.new(128, -128))
    Editor.InsertLines(vertices, true)
end

Editor.RegisterPlugin("Create Test Data", execfunc)
