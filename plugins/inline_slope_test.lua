local function execfunc()
    local vertices = {}
    table.insert(vertices, Vec2.new(-256, 96))
    table.insert(vertices, Vec2.new(480, 32))
    Editor.InsertLines(vertices, false)
    vertices = {}
    table.insert(vertices, Vec2.new(-208.13, 91.84))
    table.insert(vertices, Vec2.new(392.20, 39.63))
    Editor.InsertLines(vertices, false)
end

Editor.RegisterPlugin("InlineSlopeTest", execfunc)