#include "graphics.h"

#define _FL "graphics.c"

#define APIC static
#define GL_ADDRESS_ARR_APPEND_AMOUNT 25

APIC char* load_raw_txt(const char* fpath);
APIC GLuint compile_shader_code(const char* source, GLenum type);

void check_gl_error(const char* operation) {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        fprintf(stderr, "[%s] - OpenGL error during %s: %d\n", _FL, operation, err);
    }
}

typedef struct {
    globject_tcouple* openglobjects;
    size_t openglobjects_objcount;
    size_t openglobjects_objcapacity;
} app_resources;

void resize_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    check_gl_error("resize_callback");
}   

void error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

API void glapi_EnableDepthTest() {
    glEnable(GL_DEPTH_TEST);
}

API void glapi_DisableDepthTest() {
    glDisable(GL_DEPTH_TEST);
}

gl_app* glapi_CreateApp(uint16_t window_width, uint16_t window_height, const char* title, bool resizable, float r, float g, float b) {
    gl_app* app = (gl_app*)calloc(1, sizeof(gl_app));
    if (!app) {
        fprintf(stderr, "[%s] - Failure to allocate 'app' to heap in glapi_CreateApp\n", _FL); 
        exit(1);
    }

    app->window = (gl_window*)calloc(1, sizeof(gl_window));
    if (!app->window) {
        fprintf(stderr, "[%s] - Failure to allocate 'app->window' to heap in glapi_CreateApp\n", _FL); 
        free(app);
        exit(1);
    }

    app->resources = (void*)calloc(1, sizeof(app_resources));
    if (!app->resources) {
        fprintf(stderr, "[%s] - Failure to allocate 'app->resources' to heap in glapi_CreateApp\n", _FL); 
        free(app->window);
        free(app);
        exit(1);
    }
    app_resources* graphics_resources = (app_resources*)app->resources;
    graphics_resources->openglobjects_objcount = 0;

    glfwSetErrorCallback(error_callback);
    printf("[%s] - Starting program\n", _FL);

    int major, minor, revision;
    glfwGetVersion(&major, &minor, &revision);
    printf("[%s] - GLFW version: %d.%d.%d\n", _FL, major, minor, revision);

    printf("[%s] - Initializing GLFW\n", _FL);
    if (!glfwInit()) {
        fprintf(stderr, "[%s] - Failed to initialize GLFW\n", _FL);
        free(app->window);
        free(app->resources);
        free(app);
        exit(1);
    }
    printf("[%s] - GLFW initialized\n", _FL);

    printf("[%s] - Setting window hints\n", _FL);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif
    glfwWindowHint(GLFW_RESIZABLE, resizable ? GL_TRUE : GL_FALSE);

    printf("[%s] - Attempting to create window\n", _FL);
    app->window->pointer = glfwCreateWindow(window_width, window_height, title, NULL, NULL);
    if (!app->window->pointer) {
        fprintf(stderr, "[%s] - Failed to create window\n", _FL);
        glfwTerminate();
        free(app->window);
        free(app->resources);
        free(app);
        exit(1);
    }
    printf("[%s] - Window created\n", _FL);

    glfwMakeContextCurrent(app->window->pointer);
    printf("[%s] - Context made current\n", _FL);

    glfwSetFramebufferSizeCallback(app->window->pointer, resize_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        fprintf(stderr, "[%s] - Failed to initialize GLAD\n", _FL);
        glfwDestroyWindow(app->window->pointer);
        glfwTerminate();
        free(app->window);
        free(app->resources);
        free(app);
        exit(1);
    }
    printf("[%s] - GLAD initialized\n", _FL);
    
    const GLubyte* version = glGetString(GL_VERSION);
    if (!version) {
        fprintf(stderr, "[%s] - Failed to get OpenGL version\n", _FL);
        glfwDestroyWindow(app->window->pointer);
        glfwTerminate();
        free(app->window);
        free(app->resources);
        free(app);
        exit(1);
    }
    printf("[%s] - OpenGL version: %s\n", _FL, version);

    app->window->window_width = window_width;
    app->window->window_height = window_height;
    app->window->r = r;
    app->window->g = g;
    app->window->b = b;

    glfwShowWindow(app->window->pointer);
    printf("[%s] - Window shown\n", _FL);

    glEnable(GL_DEPTH_TEST);
    glfwSwapInterval(1);
    return app;
}

void glapi_BindApp(gl_app* app) {
    int x, y;
    glfwGetWindowSize(app->window->pointer, &x, &y);
    app->window->window_width = x;
    app->window->window_height = y;

    glClearColor(
        app->window->r, 
        app->window->g, 
        app->window->b, 
        1.0f
    );
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void glapi_UnbindApp(gl_app* app) {
    glfwSwapBuffers(app->window->pointer);
    glfwPollEvents();
}

int glapi_DestroyApp(gl_app* app) {
    app_resources* graphics_resources = (app_resources*)app->resources;
    if (graphics_resources->openglobjects) {
        globject_tcouple* clist = graphics_resources->openglobjects;
        size_t clistlen = graphics_resources->openglobjects_objcount;

        for (size_t i = 0; i < clistlen; i++) {
            if (!clist[i].globject) {
                fprintf(stderr, "[%s] - Null globject at index [%zu] in glapi_DestroyApp\n", _FL, i);
                continue;
            }
            switch (clist[i].objtype) {
                case VAO:
                    glDeleteVertexArrays(1, (GLuint*)clist[i].globject);
                    break;
                case SHADER:
                    glDeleteProgram(*(GLuint*)clist[i].globject);
                    break;
                case TEXTURE:
                    glDeleteTextures(1, (GLuint*)clist[i].globject);
                    break;
                case FRAMEBUFFER:
                    glDeleteFramebuffers(1, (GLuint*)clist[i].globject);
                    break;
                default:
                    fprintf(stderr, "[%s] - Invalid globject type [%i] in glapi_DestroyApp\n", _FL, clist[i].objtype);
                    break;
            }
        }
        graphics_resources->openglobjects_objcount = 0;
        free(clist);
    }
    
    glfwDestroyWindow(app->window->pointer);
    glfwTerminate();
    free(app->window);
    free(app->resources);
    free(app);
    return 0;
}

int glapi_ShouldAppClose(gl_app* app) {
    return glfwWindowShouldClose(app->window->pointer);
}

void glapi_AppendOpenGLObjects(gl_app* app, globject_tcouple tcouple) {
    app_resources* graphics_resources = (app_resources*)app->resources;
    globject_tcouple* clist = graphics_resources->openglobjects;
    size_t clistlen = graphics_resources->openglobjects_objcount;
    size_t clistsize = graphics_resources->openglobjects_objcapacity;

    if (!clist) {
        clistsize = GL_ADDRESS_ARR_APPEND_AMOUNT;
        clist = (globject_tcouple*)calloc(clistsize, sizeof(globject_tcouple));
        if (!clist) {
            fprintf(stderr, "[%s] - Failure to allocate 'app->resources->openglobjects' to heap in glapi_AppendOpenGLObjects\n", _FL); 
            return;
        }
        graphics_resources->openglobjects = clist;
        graphics_resources->openglobjects_objcapacity = clistsize;
    }
    else if (clistlen >= clistsize) {
        clistsize *= 2;
        globject_tcouple* new_clist = (globject_tcouple*)realloc(clist, clistsize * sizeof(globject_tcouple));
        if (!new_clist) {
            fprintf(stderr, "[%s] - Failure to reallocate 'app->resources->openglobjects' in glapi_AppendOpenGLObjects\n", _FL); 
            return;
        }
        clist = new_clist;
        graphics_resources->openglobjects = clist;
        graphics_resources->openglobjects_objcapacity = clistsize;
    }
    clist[clistlen] = tcouple;
    graphics_resources->openglobjects_objcount = clistlen + 1;
}

APIC char* load_raw_txt(const char* fpath) {
    FILE* file = fopen(fpath, "r");
    if (!file) {
        fprintf(stderr, "[%s] - Failure to read file in 'load_raw_txt(const char* fpath)': %s\n", _FL, fpath);
        return NULL;
    }  
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char* source = (char*)malloc(size + 1);
    if (!source) {
        fprintf(stderr, "[%s] - Memory allocation failed for file in 'load_raw_txt(const char* fpath)': %s\n", _FL, fpath);
        fclose(file);
        return NULL;
    }
    size_t read = fread(source, 1, size, file);
    if (read != size) {
        fprintf(stderr, "[%s] - Failure to read file in 'load_raw_txt(const char* fpath)': %s\n", _FL, fpath);
        free(source);
        fclose(file);
        return NULL;
    }
    source[size] = '\0';
    fclose(file);
    return source;
}

APIC GLuint compile_shader_code(const char* source, GLenum type) {
    GLuint shaderp = glCreateShader(type);
    glShaderSource(shaderp, 1, &source, NULL);
    glCompileShader(shaderp);
    int errcode;
    glGetShaderiv(shaderp, GL_COMPILE_STATUS, &errcode);
    if (!errcode) {
        char log[512];
        glGetShaderInfoLog(shaderp, 512, NULL, log);
        fprintf(stderr, "[%s] - Shader compilation error: %s\n", _FL, log);
    }
    return shaderp;
}

GLuint glapi_GenShaderProgram_f(gl_app* app, const char* v_source, const char* f_source, GLuint* address) {
    GLuint vshader = compile_shader_code(v_source, GL_VERTEX_SHADER);
    GLuint fshader = compile_shader_code(f_source, GL_FRAGMENT_SHADER);
    GLuint sprogram = glCreateProgram();
    glAttachShader(sprogram, vshader);
    glAttachShader(sprogram, fshader);
    glLinkProgram(sprogram);
    int errcode;
    glGetProgramiv(sprogram, GL_LINK_STATUS, &errcode);
    if (!errcode) {
        char log[512];
        glGetProgramInfoLog(sprogram, 512, NULL, log);
        fprintf(stderr, "[%s] - Shader linking error: %s\n", _FL, log);
    }
    glDeleteShader(vshader);
    glDeleteShader(fshader);
    glapi_AppendOpenGLObjects(app, T{address, SHADER});
    return sprogram;
}

GLuint glapi_GenShaderProgram_s(gl_app* app, const char* v_fpath, const char* f_fpath, GLuint* address) {
    char* v_source = load_raw_txt(v_fpath);
    char* f_source = load_raw_txt(f_fpath);
    GLuint vshader = compile_shader_code(v_source, GL_VERTEX_SHADER);
    GLuint fshader = compile_shader_code(f_source, GL_FRAGMENT_SHADER);
    GLuint sprogram = glCreateProgram();
    glAttachShader(sprogram, vshader);
    glAttachShader(sprogram, fshader);
    glLinkProgram(sprogram);
    int errcode;
    glGetProgramiv(sprogram, GL_LINK_STATUS, &errcode);
    if (!errcode) {
        char log[512];
        glGetProgramInfoLog(sprogram, 512, NULL, log);
        fprintf(stderr, "[%s] - Shader linking error: %s\n", _FL, log);
    }
    glDeleteShader(vshader);
    glDeleteShader(fshader);
    free(v_source);
    free(f_source);
    glapi_AppendOpenGLObjects(app, T{address, SHADER});
    return sprogram;
}

GLuint glapi_GenFrameBuffer(gl_app* app, gl_texture output_tex, GLuint* address, uint16_t width, uint16_t height) {
    GLuint fbuffer;
    glGenFramebuffers(1, &fbuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, fbuffer);

    glBindTexture(GL_TEXTURE_2D, output_tex);
    glTexImage2D(
        GL_TEXTURE_2D, 
        0, 
        GL_RGB, 
        width, 
        height,
        0,
        GL_RGB, 
        GL_UNSIGNED_BYTE, 
        0
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    GLuint dbuffer;
    glGenRenderbuffers(1, &dbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, dbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, dbuffer);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, output_tex, 0);

    GLenum drawbuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, drawbuffers);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "[%s] - Failure to generate framebuffer in 'glapi_GenFrameBuffer'\n", _FL);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return 0;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glapi_AppendOpenGLObjects(app, T{address, FRAMEBUFFER});
    return fbuffer;
}

GLuint glapi_GenVertexBufferObjectFromMesh(gl_app* app, gl_mesh* mesh, GLuint* address) {
    GLuint 
        vao,
        vbo_positions,
        vbo_uvs,
        vbo_normals,
        ebo;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo_positions);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_positions);
    glBufferData(GL_ARRAY_BUFFER, mesh->positions_size, mesh->positions, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    if (mesh->uvs && mesh->uvs_size > 0) {
        glGenBuffers(1, &vbo_uvs);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_uvs);
        glBufferData(GL_ARRAY_BUFFER, mesh->uvs_size, mesh->uvs, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
        glEnableVertexAttribArray(1);
    }
    
    if (mesh->normals && mesh->normals_size > 0) {
        glGenBuffers(1, &vbo_normals);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_normals);
        glBufferData(GL_ARRAY_BUFFER, mesh->normals_size, mesh->normals, GL_STATIC_DRAW);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
        glEnableVertexAttribArray(2);
    }

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices_size, mesh->indices, GL_STATIC_DRAW);

    glBindVertexArray(0);

    glapi_AppendOpenGLObjects(app, T{address, VAO});
    return vao;
}

GLuint glapi_GenTextureFromFpath(gl_app* app, const char* fpath, GLuint* address) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    int width, height, channels;
    stbi_set_flip_vertically_on_load(1);
    if (strcmp("", fpath)) {
        unsigned char* data = stbi_load(fpath, &width, &height, &channels, STBI_rgb_alpha);
        if (data) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            stbi_image_free(data);
        } else {
            fprintf(stderr, "[%s] - Texture load failed: %s\n", _FL, stbi_failure_reason());
        }
    } else {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glapi_AppendOpenGLObjects(app, T{address, TEXTURE});
    return texture;
}

void glapi_BindVertexBufferObject(gl_vao vao) {
    glBindVertexArray(vao);
}

void glapi_UnbindVertexBufferObject() {
    glBindVertexArray(0);
}

void glapi_BindShader(gl_shader shader) {
    glUseProgram(shader);
}

void glapi_UnbindShader() {
    glUseProgram(0);
}

void glapi_BindFrameBufferObject(gl_framebuffer framebuffer) {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
}

void glapi_UnbindFrameBufferObject() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void glapi_DrawVertexBufferObject(size_t isize) {
    glDrawElements(GL_TRIANGLES, isize/sizeof(GLuint), GL_UNSIGNED_INT, 0);
}

void glapi_PushIntToShader(const char* varname, int value, gl_shader shader) {
    glUseProgram(shader);
    glUniform1i(glGetUniformLocation(shader, varname), value);
}

void glapi_PushFloatToShader(const char* varname, float value, gl_shader shader) {
    glUseProgram(shader);
    glUniform1f(glGetUniformLocation(shader, varname), value);
}

void glapi_PushVec2ToShader(const char* varname, float* value, gl_shader shader) {
    glUseProgram(shader);
    glUniform2fv(glGetUniformLocation(shader, varname), 1, value);
}

void glapi_PushVec3ToShader(const char* varname, float* value, gl_shader shader) {
    glUseProgram(shader);
    glUniform3fv(glGetUniformLocation(shader, varname), 1, value);
}

void glapi_PushVec4ToShader(const char* varname, float* value, gl_shader shader) {
    glUseProgram(shader);
    glUniform4fv(glGetUniformLocation(shader, varname), 1, value);
}

void glapi_PushMatrix3x3ToShader(const char* varname, float* value, gl_shader shader) {
    glUseProgram(shader);
    glUniformMatrix3fv(glGetUniformLocation(shader, varname), 1, GL_FALSE, value);
}

void glapi_PushMatrix4x4ToShader(const char* varname, float* value, gl_shader shader) {
    glUseProgram(shader);
    glUniformMatrix4fv(glGetUniformLocation(shader, varname), 1, GL_FALSE, value);
}

void glapi_PushTexture2DToShader(const char* varname, gl_texture value, gl_shader shader) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, value);
    glUniform1i(glGetUniformLocation(shader, varname), 0);
}