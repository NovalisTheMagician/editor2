#include "editor.h"

#define BUFFER_SIZE 10000

static bool CompileShader(const char *shaderScr, GLuint *shader)
{
    int success;
    char infoLog[512];

    glShaderSource(*shader, 1, &shaderScr, NULL);
    glCompileShader(*shader);
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(*shader, sizeof infoLog, NULL, infoLog);
        printf("Failed to compile Shader: %s\n", infoLog);
        return false;
    };

    return true;
}

static bool LinkProgram(GLuint vertexShader, GLuint fragmentShader, GLuint *program)
{
    int success;
    char infoLog[512];

    glAttachShader(*program, vertexShader);
    glAttachShader(*program, fragmentShader);
    glLinkProgram(*program);
    glGetProgramiv(*program, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(*program, sizeof infoLog, NULL, infoLog);
        printf("Failed to link Program: %s\n", infoLog);
        return false;
    }

    return true;
}

static bool InitBackground(struct EdState *state)
{
    const char *hVertShaderSrc = 
        "#version 460 core\n"
        "layout(location=0) in vec2 inPosition;\n"
        "uniform mat4 viewProj;\n"
        "uniform float offset;\n"
        "uniform float period;\n"
        "void main() {\n"
        "   float off = period * gl_InstanceID + offset;\n"
        "   vec2 pos = inPosition + vec2(0, off);\n"
        "   gl_Position = viewProj * vec4(pos, 0, 1);\n"
        "}\n";

    const char *vVertShaderSrc = 
        "#version 460 core\n"
        "layout(location=0) in vec2 inPosition;\n"
        "uniform mat4 viewProj;\n"
        "uniform float offset;\n"
        "uniform float period;\n"
        "void main() {\n"
        "   float off = period * gl_InstanceID + offset;\n"
        "   vec2 pos = inPosition + vec2(off, 0);\n"
        "   gl_Position = viewProj * vec4(pos, 0, 1);\n"
        "}\n";

    const char *fragShaderSrc = 
        "#version 460 core\n"
        "uniform vec4 tint;\n"
        "out vec4 fragColor;\n"
        "void main() {\n"
        "   fragColor = tint;\n"
        "}\n";

    GLuint hVertShader = glCreateShader(GL_VERTEX_SHADER);
    if(!CompileShader(hVertShaderSrc, &hVertShader))
        return false;

    GLuint vVertShader = glCreateShader(GL_VERTEX_SHADER);
    if(!CompileShader(vVertShaderSrc, &vVertShader))
        return false;

    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    if(!CompileShader(fragShaderSrc, &fragShader))
        return false;

    GLuint hProg = glCreateProgram();
    if(!LinkProgram(hVertShader, fragShader, &hProg))
        return false;

    GLuint vProg = glCreateProgram();
    if(!LinkProgram(vVertShader, fragShader, &vProg))
        return false;

    glDeleteShader(hVertShader);
    glDeleteShader(vVertShader);
    glDeleteShader(fragShader);

    state->gl.editorBackProg.hProgram = hProg;
    state->gl.editorBackProg.vProgram = vProg;

    GLuint vao;
    glCreateVertexArrays(1, &vao);
    glEnableVertexArrayAttrib(vao, 0);
    glVertexArrayAttribFormat(vao, 0, 2, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(vao, 0, 0);

    state->gl.editorBackProg.backVertexFormat = vao;

    GLuint lineBuffer;
    glCreateBuffers(1, &lineBuffer);
    glNamedBufferStorage(lineBuffer, 4 * sizeof(vec2), NULL, GL_DYNAMIC_STORAGE_BIT);

    glVertexArrayVertexBuffer(vao, 0, lineBuffer, 0, sizeof(vec2));

    state->gl.backgroundLinesBuffer = lineBuffer;

    state->gl.editorBackProg.hOffsetUniform = glGetUniformLocation(hProg, "offset");
    state->gl.editorBackProg.hPeriodUniform = glGetUniformLocation(hProg, "period");
    state->gl.editorBackProg.hTintUniform = glGetUniformLocation(hProg, "tint");
    state->gl.editorBackProg.hVPUniform = glGetUniformLocation(hProg, "viewProj");

    state->gl.editorBackProg.vOffsetUniform = glGetUniformLocation(vProg, "offset");
    state->gl.editorBackProg.vPeriodUniform = glGetUniformLocation(vProg, "period");
    state->gl.editorBackProg.vTintUniform = glGetUniformLocation(vProg, "tint");
    state->gl.editorBackProg.vVPUniform = glGetUniformLocation(vProg, "viewProj");

    return true;
}

static bool InitVertex(struct EdState *state)
{
    const char *vertShaderSrc = 
        "#version 460 core\n"
        "layout(location=0) in vec2 inPosition;\n"
        "layout(location=1) in vec4 inColor;\n"
        "out vec4 outColor;\n"
        "uniform mat4 viewProj;\n"
        "void main() {\n"
        "   gl_Position = viewProj * vec4(inPosition, 0, 1);\n"
        "   outColor = inColor;\n"
        "}\n";

    const char *fragShaderSrc = 
        "#version 460 core\n"
        "in vec4 outColor;"
        "out vec4 fragColor;\n"
        "uniform vec4 tint;\n"
        "void main() {\n"
        "   fragColor = outColor;\n"
        "}\n";

    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    if(!CompileShader(vertShaderSrc, &vertShader))
        return false;

    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    if(!CompileShader(fragShaderSrc, &fragShader))
        return false;

    GLuint program = glCreateProgram();
    if(!LinkProgram(vertShader, fragShader, &program))
        return false;

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    state->gl.editorVertex.program = program;
    state->gl.editorVertex.viewProjUniform = glGetUniformLocation(program, "viewProj");

    const GLbitfield 
	mapping_flags = GL_MAP_WRITE_BIT | GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT,
	storage_flags = GL_DYNAMIC_STORAGE_BIT | mapping_flags;

    size_t bufferSize = BUFFER_SIZE * sizeof(struct VertexType);
    GLuint buffer;
    glCreateBuffers(1, &buffer);
    glNamedBufferStorage(buffer, bufferSize, NULL, storage_flags);

    state->gl.editorVertex.vertBuffer = buffer;

    GLuint vao;
    glCreateVertexArrays(1, &vao);
    glEnableVertexArrayAttrib(vao, 0);
    glEnableVertexArrayAttrib(vao, 1);
    glVertexArrayAttribIFormat(vao, 0, 2, GL_INT, offsetof(struct VertexType, x));
    glVertexArrayAttribFormat(vao, 1, 4, GL_FLOAT, GL_FALSE, offsetof(struct VertexType, color));
    glVertexArrayAttribBinding(vao, 0, 0);
    glVertexArrayAttribBinding(vao, 1, 0);

    glVertexArrayVertexBuffer(vao, 0, buffer, 0, sizeof(struct VertexType));

    state->gl.editorVertex.vertFormat = vao;


    state->gl.editorVertex.bufferMap = glMapNamedBufferRange(buffer, 0, bufferSize, mapping_flags);

    return true;
}

bool LoadShaders(struct EdState *state)
{
    if(!InitBackground(state))
        return false;

    if(!InitVertex(state))
        return false;

    return true;
}
