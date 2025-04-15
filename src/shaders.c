#include "editor.h"

#include "resources/resources.h"

static bool CompileShader(const char *shaderScr, GLuint *shader, char *error, size_t errorSize)
{
    int success;
    char infoLog[512];

    glShaderSource(*shader, 1, &shaderScr, NULL);
    glCompileShader(*shader);
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(*shader, sizeof infoLog, NULL, infoLog);
        snprintf(error, errorSize, "Failed to compile Shader: %s\n", infoLog);
        return false;
    };

    return true;
}

static bool LinkProgram(GLuint vertexShader, GLuint fragmentShader, GLuint *program, char *error, size_t errorSize)
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
        snprintf(error, errorSize, "Failed to link Program: %s\n", infoLog);
        return false;
    }

    return true;
}

static bool InitBackground(EdState *state, char *error, size_t errorSize)
{
    GLuint hVertShader = glCreateShader(GL_VERTEX_SHADER);
    if(!CompileShader(gBackH_vsData, &hVertShader, error, errorSize))
        return false;

    GLuint vVertShader = glCreateShader(GL_VERTEX_SHADER);
    if(!CompileShader(gBackV_vsData, &vVertShader, error, errorSize))
        return false;

    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    if(!CompileShader(gBack_fsData, &fragShader, error, errorSize))
        return false;

    GLuint hProg = glCreateProgram();
    if(!LinkProgram(hVertShader, fragShader, &hProg, error, errorSize))
        return false;

    GLuint vProg = glCreateProgram();
    if(!LinkProgram(vVertShader, fragShader, &vProg, error, errorSize))
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

    GLuint shaderDataBuffer;
    glCreateBuffers(1, &shaderDataBuffer);
    glNamedBufferStorage(shaderDataBuffer, sizeof(BackgroundShaderData), NULL, GL_DYNAMIC_STORAGE_BIT);
    state->gl.editorBackProg.shaderDataBuffer = shaderDataBuffer;

    return true;
}

static bool InitVertex(EdState *state, char *error, size_t errorSize)
{
    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    if(!CompileShader(gVertex_vsData, &vertShader, error, errorSize))
        return false;

    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    if(!CompileShader(gVertex_fsData, &fragShader, error, errorSize))
        return false;

    GLuint program = glCreateProgram();
    if(!LinkProgram(vertShader, fragShader, &program, error, errorSize))
        return false;

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    state->gl.editorVertex.program = program;

    return true;
}

static bool InitLines(EdState *state, char *error, size_t errorSize)
{
    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    if(!CompileShader(gLine_vsData, &vertShader, error, errorSize))
        return false;

    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    if(!CompileShader(gLine_fsData, &fragShader, error, errorSize))
        return false;

    GLuint program = glCreateProgram();
    if(!LinkProgram(vertShader, fragShader, &program, error, errorSize))
        return false;

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    state->gl.editorLine.program = program;

    return true;
}

static bool InitSectors(EdState *state, char *error, size_t errorSize)
{
    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    if(!CompileShader(gSector_vsData, &vertShader, error, errorSize))
        return false;

    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    if(!CompileShader(gSector_fsData, &fragShader, error, errorSize))
        return false;

    GLuint program = glCreateProgram();
    if(!LinkProgram(vertShader, fragShader, &program, error, errorSize))
        return false;

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    state->gl.editorSector.program = program;

    return true;
}

static bool InitRealtime(EdState *state, char *error, size_t errorSize)
{
    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    if(!CompileShader(gRealtime_vsData, &vertShader, error, errorSize))
        return false;

    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    if(!CompileShader(gRealtime_fsData, &fragShader, error, errorSize))
        return false;

    GLuint program = glCreateProgram();
    if(!LinkProgram(vertShader, fragShader, &program, error, errorSize))
        return false;

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    state->gl.realtimeProgram.program = program;

    return true;
}

bool LoadShaders(EdState *state, char *error, size_t errorSize)
{
    if(!InitBackground(state, error, errorSize))
        return false;

    if(!InitVertex(state, error, errorSize))
        return false;

    if(!InitLines(state, error, errorSize))
        return false;

    if(!InitSectors(state, error, errorSize))
        return false;

    if(!InitRealtime(state, error, errorSize))
        return false;

    return true;
}
