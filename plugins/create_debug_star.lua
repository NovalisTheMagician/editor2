local function execfunc()
    local vertices = {}
    table.insert(vertices, Vec2.new(-96, 64))
    table.insert(vertices, Vec2.new(-32, -128))
    table.insert(vertices, Vec2.new(32, 64))
    table.insert(vertices, Vec2.new(-128, -64))
    table.insert(vertices, Vec2.new(64, -64))
    Editor.InsertLines(vertices, true)
end

Editor.RegisterPlugin("Create Test Star", execfunc)
