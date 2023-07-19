#include "editor.h"

static void message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const* message, void const* user_param)
{
    const char *src_str = ({
        char *v = "";
		switch (source) {
		case GL_DEBUG_SOURCE_API: v = "API"; break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM: v = "WINDOW SYSTEM"; break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: v = "SHADER COMPILER"; break;
		case GL_DEBUG_SOURCE_THIRD_PARTY: v = "THIRD PARTY"; break;
		case GL_DEBUG_SOURCE_APPLICATION: v = "APPLICATION"; break;
		case GL_DEBUG_SOURCE_OTHER: v = "OTHER"; break;
		}
        v;
	});

	const char *type_str = ({
        char *v = "";
		switch (type) {
		case GL_DEBUG_TYPE_ERROR: v = "ERROR"; break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: v = "DEPRECATED_BEHAVIOR"; break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: v = "UNDEFINED_BEHAVIOR"; break;
		case GL_DEBUG_TYPE_PORTABILITY: v = "PORTABILITY"; break;
		case GL_DEBUG_TYPE_PERFORMANCE: v = "PERFORMANCE"; break;
		case GL_DEBUG_TYPE_MARKER: v = "MARKER"; break;
		case GL_DEBUG_TYPE_OTHER: v = "OTHER"; break;
		}
        v;
	});

	const char *severity_str = ({
        char *v = "";
		switch (severity) {
		case GL_DEBUG_SEVERITY_NOTIFICATION: v = "NOTIFICATION"; break;
		case GL_DEBUG_SEVERITY_LOW: v = "LOW"; break;
		case GL_DEBUG_SEVERITY_MEDIUM: v = "MEDIUM"; break;
		case GL_DEBUG_SEVERITY_HIGH: v = "HIGH"; break;
		}
        v;
	});
    printf("%s, %s, %s, %u: %s\n", src_str, type_str, severity_str, id, message);
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

    int success;
    char infoLog[512];

    GLuint hVertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(hVertShader, 1, &hVertShaderSrc, NULL);
    glCompileShader(hVertShader);
    glGetShaderiv(hVertShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(hVertShader, sizeof infoLog, NULL, infoLog);
        printf("Failed to compile hVertex Shader: %s\n", infoLog);
        return false;
    };

    GLuint vVertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vVertShader, 1, &vVertShaderSrc, NULL);
    glCompileShader(vVertShader);
    glGetShaderiv(vVertShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(vVertShader, sizeof infoLog, NULL, infoLog);
        printf("Failed to compile vVertex Shader: %s\n", infoLog);
        return false;
    };

    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &fragShaderSrc, NULL);
    glCompileShader(fragShader);
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(fragShader, sizeof infoLog, NULL, infoLog);
        printf("Failed to compile fragment Shader: %s\n", infoLog);
        return false;
    };

    GLuint hProg = glCreateProgram();
    glAttachShader(hProg, hVertShader);
    glAttachShader(hProg, fragShader);
    glLinkProgram(hProg);
    glGetProgramiv(hProg, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(hProg, sizeof infoLog, NULL, infoLog);
        printf("Failed to link hProg: %s\n", infoLog);
        return false;
    }

    GLuint vProg = glCreateProgram();
    glAttachShader(vProg, vVertShader);
    glAttachShader(vProg, fragShader);
    glLinkProgram(vProg);
    glGetProgramiv(vProg, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(vProg, sizeof infoLog, NULL, infoLog);
        printf("Failed to link vProg: %s\n", infoLog);
        return false;
    }

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

bool InitEditor(struct EdState *state)
{
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(message_callback, NULL);

    glCreateFramebuffers(1, &state->gl.editorFramebuffer);
    glCreateFramebuffers(1, &state->gl.realtimeFramebuffer);

    state->ui.gridSize = 32;
    state->ui.zoomLevel = 1.0f;

    if(!InitBackground(state))
        return false;

    return true;
}

void DestroyEditor(struct EdState *state)
{
    glDeleteBuffers(1, &state->gl.backgroundLinesBuffer);
    glDeleteVertexArrays(1, &state->gl.editorBackProg.backVertexFormat);
    glDeleteProgram(state->gl.editorBackProg.hProgram);
    glDeleteProgram(state->gl.editorBackProg.vProgram);
    glDeleteFramebuffers(2, (GLuint[]){ state->gl.editorFramebuffer, state->gl.realtimeFramebuffer });
    glDeleteTextures(3, (GLuint[])
                                { 
                                    state->gl.editorColorTexture,
                                    state->gl.realtimeColorTexture, 
                                    state->gl.realtimeDepthTexture
                                });
}

void ResizeEditorView(struct EdState *state, int width, int height)
{
    if(width == 0 || height == 0)
        return;

    if(state->gl.editorFramebufferWidth == width && state->gl.editorFramebufferHeight == height)
        return;

    if(state->gl.editorColorTexture > 0)
    {
        glDeleteTextures(1, &state->gl.editorColorTexture);
    }

    glCreateTextures(GL_TEXTURE_2D, 1, &state->gl.editorColorTexture);
    glTextureStorage2D(state->gl.editorColorTexture, 1, GL_RGBA8, width, height);
    glTextureParameteri(state->gl.editorColorTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(state->gl.editorColorTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glNamedFramebufferTexture(state->gl.editorFramebuffer, GL_COLOR_ATTACHMENT0, state->gl.editorColorTexture, 0);

    state->gl.editorFramebufferWidth = width;
    state->gl.editorFramebufferHeight = height;

    vec2 horLine[] = {{ 0, 0 }, { width, 0 }};
    vec2 verLine[] = {{ 0, 0 }, { 0, height }};
    glNamedBufferSubData(state->gl.backgroundLinesBuffer, 0, 2 * sizeof(vec2), horLine);
    glNamedBufferSubData(state->gl.backgroundLinesBuffer, 2 * sizeof(vec2), 2 * sizeof(vec2), verLine);

    glm_ortho(0, width, 0, height, -1, 1, state->editorProjection);
}

void ResizeRealtimeView(struct EdState *state, int width, int height)
{
    if(width == 0 || height == 0)
        return;

    if(state->gl.realtimeFramebufferWidth == width && state->gl.realtimeFramebufferHeight == height)
        return;

    if(state->gl.realtimeColorTexture > 0)
    {
        glDeleteTextures(2, (GLuint[]){ state->gl.realtimeColorTexture, state->gl.realtimeDepthTexture });
    }

    glCreateTextures(GL_TEXTURE_2D, 1, &state->gl.realtimeColorTexture);
    glTextureStorage2D(state->gl.realtimeColorTexture, 1, GL_RGBA8, width, height);
    glTextureParameteri(state->gl.realtimeColorTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(state->gl.realtimeColorTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glCreateTextures(GL_TEXTURE_2D, 1, &state->gl.realtimeDepthTexture);
    glTextureStorage2D(state->gl.realtimeDepthTexture, 1, GL_DEPTH24_STENCIL8, width, height);
    glTextureParameteri(state->gl.realtimeDepthTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(state->gl.realtimeDepthTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glNamedFramebufferTexture(state->gl.realtimeFramebuffer, GL_COLOR_ATTACHMENT0, state->gl.realtimeColorTexture, 0);
    glNamedFramebufferTexture(state->gl.realtimeFramebuffer, GL_DEPTH_STENCIL_ATTACHMENT, state->gl.realtimeDepthTexture, 0);

    state->gl.realtimeFramebufferWidth = width;
    state->gl.realtimeFramebufferHeight = height;
}

void RenderEditorView(const struct EdState *state, const struct Map *map)
{
    float period = state->ui.gridSize;

    int vLines = (state->gl.editorFramebufferWidth / period) + 1;
    int hLines = (state->gl.editorFramebufferHeight / period) + 1;

    glBindVertexArray(state->gl.editorBackProg.backVertexFormat);
    glUseProgram(state->gl.editorBackProg.hProgram);
    glUniform1f(state->gl.editorBackProg.hOffsetUniform, state->backOffset[1]);
    glUniform1f(state->gl.editorBackProg.hPeriodUniform, period);
    glUniform4fv(state->gl.editorBackProg.hTintUniform, 1, state->settings.colors[COL_BACK_LINES]);
    glUniformMatrix4fv(state->gl.editorBackProg.hVPUniform, 1, false, (float*)state->editorProjection);

    glDrawArraysInstanced(GL_LINES, 0, 2, hLines);

    glUseProgram(state->gl.editorBackProg.vProgram);
    glUniform1f(state->gl.editorBackProg.vOffsetUniform, state->backOffset[0]);
    glUniform1f(state->gl.editorBackProg.vPeriodUniform, period);
    glUniform4fv(state->gl.editorBackProg.vTintUniform, 1, state->settings.colors[COL_BACK_LINES]);
    glUniformMatrix4fv(state->gl.editorBackProg.vVPUniform, 1, false, (float*)state->editorProjection);

    glDrawArraysInstanced(GL_LINES, 2, 2, vLines);
}

void RenderRealtimeView(const struct EdState *state, const struct Map *map)
{

}
