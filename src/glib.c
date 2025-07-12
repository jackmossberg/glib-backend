#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stdbool.h>
#include <stdint.h>
#include "graphics.h"
#include "maths.h"

#define _FL "graphics.c"

static PyObject* glib_depth_test(PyObject* self, PyObject* args);
static PyObject* glib_get_window_width(PyObject* self, PyObject* args);
static PyObject* glib_get_window_height(PyObject* self, PyObject* args);
static PyObject* glib_degs_to_rads(PyObject* self, PyObject* args);
static PyObject* glib_rads_to_degs(PyObject* self, PyObject* args);
static PyObject* glib_vec2_add(PyObject* self, PyObject* args);
static PyObject* glib_vec3_add(PyObject* self, PyObject* args);
static PyObject* glib_vec4_add(PyObject* self, PyObject* args);
static PyObject* glib_matrix4x4_multi(PyObject* self, PyObject* args);
static PyObject* glib_matrix4x4_fill(PyObject* self, PyObject* args);
static PyObject* glib_transform_matrix4x4(PyObject* self, PyObject* args);
static PyObject* glib_view_matrix4x4(PyObject* self, PyObject* args);
static PyObject* glib_projection_matrix4x4(PyObject* self, PyObject* args);
static PyObject* glib_create_app(PyObject* self, PyObject* args);
static PyObject* glib_bind_app(PyObject* self, PyObject* args);
static PyObject* glib_unbind_app(PyObject* self, PyObject* args);
static PyObject* glib_destroy_app(PyObject* self, PyObject* args);
static PyObject* glib_should_app_close(PyObject* self, PyObject* args);
static PyObject* glib_bind_vertex_buffer_object(PyObject* self, PyObject* args);
static PyObject* glib_unbind_vertex_buffer_object(PyObject* self, PyObject* args);
static PyObject* glib_bind_shader(PyObject* self, PyObject* args);
static PyObject* glib_unbind_shader(PyObject* self, PyObject* args);
static PyObject* glib_bind_frame_buffer_object(PyObject* self, PyObject* args);
static PyObject* glib_unbind_frame_buffer_object(PyObject* self, PyObject* args);
static PyObject* glib_draw_vertex_buffer_object(PyObject* self, PyObject* args);
static PyObject* glib_gen_shader_program_f(PyObject* self, PyObject* args);
static PyObject* glib_gen_shader_program_s(PyObject* self, PyObject* args);
static PyObject* glib_gen_vertex_buffer_object(PyObject* self, PyObject* args);
static PyObject* glib_gen_frame_buffer_object(PyObject* self, PyObject* args);
static PyObject* glib_gen_texture_from_fpath(PyObject* self, PyObject* args);
static PyObject* glib_push_int_to_shader(PyObject* self, PyObject* args);
static PyObject* glib_push_float_to_shader(PyObject* self, PyObject* args);
static PyObject* glib_push_vec2_to_shader(PyObject* self, PyObject* args);
static PyObject* glib_push_vec3_to_shader(PyObject* self, PyObject* args);
static PyObject* glib_push_vec4_to_shader(PyObject* self, PyObject* args);
static PyObject* glib_push_matrix3x3_to_shader(PyObject* self, PyObject* args);
static PyObject* glib_push_matrix4x4_to_shader(PyObject* self, PyObject* args);
static PyObject* glib_push_texture2D_to_shader(PyObject* self, PyObject* args);

static int sequence_to_float_array(PyObject* seq, float* arr, Py_ssize_t expected_len) {
    if (!PySequence_Check(seq) || PySequence_Length(seq) != expected_len) {
        PyErr_SetString(PyExc_ValueError, "Expected a sequence of correct length");
        return 0;
    }
    for (Py_ssize_t i = 0; i < expected_len; i++) {
        PyObject* item = PySequence_GetItem(seq, i);
        if (!item || !PyFloat_Check(item)) {
            PyErr_SetString(PyExc_ValueError, "Expected float values in sequence");
            Py_XDECREF(item);
            return 0;
        }
        arr[i] = (float)PyFloat_AsDouble(item);
        Py_DECREF(item);
    }
    return 1;
}

static PyObject* float_array_to_list(float* arr, Py_ssize_t len) {
    PyObject* list = PyList_New(len);
    if (!list) {
        PyErr_SetString(PyExc_MemoryError, "Failed to allocate list");
        return NULL;
    }
    for (Py_ssize_t i = 0; i < len; i++) {
        PyObject* item = PyFloat_FromDouble((double)arr[i]);
        if (!item) {
            Py_DECREF(list);
            return NULL;
        }
        PyList_SetItem(list, i, item);
    }
    return list;
}

static PyObject* glib_vec2_add(PyObject* self, PyObject* args) {
    PyObject *v1_obj, *v2_obj;
    float v1[2], v2[2], result[2];

    if (!PyArg_ParseTuple(args, "OO", &v1_obj, &v2_obj)) {
        PyErr_SetString(PyExc_TypeError, "Expected two sequences");
        return NULL;
    }

    if (!sequence_to_float_array(v1_obj, v1, 2) || !sequence_to_float_array(v2_obj, v2, 2)) {
        return NULL;
    }

    result[0] = v1[0] + v2[0];
    result[1] = v1[1] + v2[1];

    return float_array_to_list(result, 2);
}

static PyObject* glib_vec3_add(PyObject* self, PyObject* args) {
    PyObject *v1_obj, *v2_obj;
    float v1[3], v2[3], result[3];

    if (!PyArg_ParseTuple(args, "OO", &v1_obj, &v2_obj)) {
        PyErr_SetString(PyExc_TypeError, "Expected two sequences");
        return NULL;
    }

    if (!sequence_to_float_array(v1_obj, v1, 3) || !sequence_to_float_array(v2_obj, v2, 3)) {
        return NULL;
    }

    result[0] = v1[0] + v2[0];
    result[1] = v1[1] + v2[1];
    result[2] = v1[2] + v2[2];

    return float_array_to_list(result, 3);
}

static PyObject* glib_vec4_add(PyObject* self, PyObject* args) {
    PyObject *v1_obj, *v2_obj;
    float v1[4], v2[4], result[4];

    if (!PyArg_ParseTuple(args, "OO", &v1_obj, &v2_obj)) {
        PyErr_SetString(PyExc_TypeError, "Expected two sequences");
        return NULL;
    }

    if (!sequence_to_float_array(v1_obj, v1, 4) || !sequence_to_float_array(v2_obj, v2, 4)) {
        return NULL;
    }

    result[0] = v1[0] + v2[0];
    result[1] = v1[1] + v2[1];
    result[2] = v1[2] + v2[2];
    result[3] = v1[3] + v2[3];

    return float_array_to_list(result, 4);
}

static PyObject* glib_matrix4x4_multi(PyObject* self, PyObject* args) {
    PyObject *m1_obj, *m2_obj;
    float m1[16], m2[16], result[16];

    if (!PyArg_ParseTuple(args, "OO", &m1_obj, &m2_obj)) {
        PyErr_SetString(PyExc_TypeError, "Expected two sequences");
        return NULL;
    }

    if (!sequence_to_float_array(m1_obj, m1, 16) || !sequence_to_float_array(m2_obj, m2, 16)) {
        return NULL;
    }

    mapi_MultiMatrix4x4(m1, m2, result);

    return float_array_to_list(result, 16);
}

static PyObject* glib_matrix4x4_fill(PyObject* self, PyObject* args) {
    float val;

    if (!PyArg_ParseTuple(args, "f", &val)) {
        PyErr_SetString(PyExc_TypeError, "Expected a float value");
        return NULL;
    }

    float matrix3x3[9];
    mapi_Matrix3x3Fill(matrix3x3, val);

    float result[16];
    for (int i = 0; i < 16; i++) result[i] = (i % 5 == 3) ? 0.0f : 1.0f;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            result[i * 4 + j] = matrix3x3[i * 3 + j];

    return float_array_to_list(result, 16);
}

static PyObject* glib_transform_matrix4x4(PyObject* self, PyObject* args) {
    PyObject *pos_obj, *rot_obj, *scale_obj;
    float pos[3], rot[3], scale[3], result[16];

    if (!PyArg_ParseTuple(args, "OOO", &pos_obj, &rot_obj, &scale_obj)) {
        PyErr_SetString(PyExc_TypeError, "Expected three sequences");
        return NULL;
    }

    if (!sequence_to_float_array(pos_obj, pos, 3) ||
        !sequence_to_float_array(rot_obj, rot, 3) ||
        !sequence_to_float_array(scale_obj, scale, 3)) {
        return NULL;
    }

    mapi_TransformMatrix4x4(result, pos, rot, scale);

    return float_array_to_list(result, 16);
}

static PyObject* glib_view_matrix4x4(PyObject* self, PyObject* args) {
    PyObject *pos_obj, *rot_obj;
    float pos[3], rot[3], result[16];

    if (!PyArg_ParseTuple(args, "OO", &pos_obj, &rot_obj)) {
        PyErr_SetString(PyExc_TypeError, "Expected two sequences");
        return NULL;
    }

    if (!sequence_to_float_array(pos_obj, pos, 3) ||
        !sequence_to_float_array(rot_obj, rot, 3)) {
        return NULL;
    }

    mapi_ViewMatrix4x4(result, pos, rot);

    return float_array_to_list(result, 16);
}

static PyObject* glib_projection_matrix4x4(PyObject* self, PyObject* args) {
    float fovy, width, height;

    if (!PyArg_ParseTuple(args, "fff", &fovy, &width, &height)) {
        PyErr_SetString(PyExc_TypeError, "Expected three float values");
        return NULL;
    }

    float result[16];
    mapi_ProjectionMatrix4x4(result, fovy, width, height);

    return float_array_to_list(result, 16);
}

static PyObject* glib_gen_vertex_buffer_object(PyObject* self, PyObject* args) {
    PyObject *app_capsule, *positions, *indices, *uvs = NULL, *normals = NULL;

    if (!PyArg_ParseTuple(args, "OOO|OO", &app_capsule, &positions, &indices, &uvs, &normals)) {
        PyErr_SetString(PyExc_TypeError, "Expected app, positions, indices, [uvs, normals]");
        return NULL;
    }

    gl_app* app = (gl_app*)PyCapsule_GetPointer(app_capsule, "gl_app");
    if (!app) {
        PyErr_SetString(PyExc_ValueError, "Invalid gl_app pointer");
        return NULL;
    }

    if (!PySequence_Check(positions) || !PySequence_Check(indices) ||
        (uvs && uvs != Py_None && !PySequence_Check(uvs)) ||
        (normals && normals != Py_None && !PySequence_Check(normals))) {
        PyErr_SetString(PyExc_TypeError, "Inputs must be sequences");
        return NULL;
    }

    Py_ssize_t pos_len = PySequence_Length(positions);
    Py_ssize_t idx_len = PySequence_Length(indices);
    Py_ssize_t uvs_len = (uvs && uvs != Py_None) ? PySequence_Length(uvs) : 0;
    Py_ssize_t norm_len = (normals && normals != Py_None) ? PySequence_Length(normals) : 0;

    if (pos_len % 3 != 0) {
        PyErr_SetString(PyExc_ValueError, "Positions sequence size must be a multiple of 3");
        return NULL;
    }
    if (idx_len % 3 != 0) {
        PyErr_SetString(PyExc_ValueError, "Indices sequence size must be a multiple of 3");
        return NULL;
    }
    if (uvs_len > 0 && uvs_len % 2 != 0) {
        PyErr_SetString(PyExc_ValueError, "UVs sequence size must be a multiple of 2");
        return NULL;
    }
    if (norm_len > 0 && norm_len % 3 != 0) {
        PyErr_SetString(PyExc_ValueError, "Normals sequence size must be a multiple of 3");
        return NULL;
    }

    float* pos_data = (float*)malloc(pos_len * sizeof(float));
    GLuint* idx_data = (GLuint*)malloc(idx_len * sizeof(GLuint));
    float* uvs_data = uvs_len > 0 ? (float*)malloc(uvs_len * sizeof(float)) : NULL;
    float* norm_data = norm_len > 0 ? (float*)malloc(norm_len * sizeof(float)) : NULL;

    if (!pos_data || !idx_data || (uvs_len > 0 && !uvs_data) || (norm_len > 0 && !norm_data)) {
        free(pos_data);
        free(idx_data);
        free(uvs_data);
        free(norm_data);
        PyErr_SetString(PyExc_MemoryError, "Failed to allocate memory");
        return NULL;
    }

    for (Py_ssize_t i = 0; i < pos_len; i++) {
        PyObject* item = PySequence_GetItem(positions, i);
        if (!PyFloat_Check(item)) {
            free(pos_data);
            free(idx_data);
            free(uvs_data);
            free(norm_data);
            PyErr_SetString(PyExc_TypeError, "Positions must contain floats");
            Py_DECREF(item);
            return NULL;
        }
        pos_data[i] = (float)PyFloat_AsDouble(item);
        Py_DECREF(item);
    }

    for (Py_ssize_t i = 0; i < idx_len; i++) {
        PyObject* item = PySequence_GetItem(indices, i);
        if (!PyLong_Check(item)) {
            free(pos_data);
            free(idx_data);
            free(uvs_data);
            free(norm_data);
            PyErr_SetString(PyExc_TypeError, "Indices must contain integers");
            Py_DECREF(item);
            return NULL;
        }
        idx_data[i] = (GLuint)PyLong_AsUnsignedLong(item);
        Py_DECREF(item);
    }

    if (uvs_len > 0) {
        for (Py_ssize_t i = 0; i < uvs_len; i++) {
            PyObject* item = PySequence_GetItem(uvs, i);
            if (!PyFloat_Check(item)) {
                free(pos_data);
                free(idx_data);
                free(uvs_data);
                free(norm_data);
                PyErr_SetString(PyExc_TypeError, "UVs must contain floats");
                Py_DECREF(item);
                return NULL;
            }
            uvs_data[i] = (float)PyFloat_AsDouble(item);
            Py_DECREF(item);
        }
    }

    if (norm_len > 0) {
        for (Py_ssize_t i = 0; i < norm_len; i++) {
            PyObject* item = PySequence_GetItem(normals, i);
            if (!PyFloat_Check(item)) {
                free(pos_data);
                free(idx_data);
                free(uvs_data);
                free(norm_data);
                PyErr_SetString(PyExc_TypeError, "Normals must contain floats");
                Py_DECREF(item);
                return NULL;
            }
            norm_data[i] = (float)PyFloat_AsDouble(item);
            Py_DECREF(item);
        }
    }

    gl_mesh mesh = {0};
    mesh.positions = pos_data;
    mesh.indices = idx_data;
    mesh.positions_size = pos_len * sizeof(float);
    mesh.indices_size = idx_len * sizeof(GLuint);
    mesh.uvs = uvs_data;
    mesh.uvs_size = uvs_len * sizeof(float);
    mesh.normals = norm_data;
    mesh.normals_size = norm_len * sizeof(float);

    GLuint vao_address;
    GLuint vao = glapi_GenVertexBufferObjectFromMesh(app, &mesh, &vao_address);

    free(pos_data);
    free(idx_data);
    free(uvs_data);
    free(norm_data);

    if (!vao) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to generate vertex buffer object");
        return NULL;
    }

    return PyLong_FromUnsignedLong(vao);
}

static PyObject* glib_gen_frame_buffer_object(PyObject* self, PyObject* args) {
    PyObject* app_capsule;
    int out_tex, width, height;

    if (!PyArg_ParseTuple(args, "Oiii", &app_capsule, &out_tex, &width, &height)) {
        PyErr_SetString(PyExc_ValueError, "Expected a tuple of type 'Oiii' (app_capsule, out_tex, width, height)");
        return NULL;
    }

    gl_app* app = (gl_app*)PyCapsule_GetPointer(app_capsule, "gl_app");
    if (!app) {
        PyErr_SetString(PyExc_ValueError, "Invalid gl_app pointer");
        return NULL;
    }

    GLuint fb_address;
    GLuint framebuffer = glapi_GenFrameBuffer(app, out_tex, &fb_address, width, height);

    if (!framebuffer) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to generate frame buffer object");
        return NULL;
    }

    return PyLong_FromUnsignedLong(framebuffer);
}

static PyObject* glib_bind_frame_buffer_object(PyObject* self, PyObject* args) {
    gl_framebuffer framebuffer;
    if (!PyArg_ParseTuple(args, "i", &framebuffer)) {
        PyErr_SetString(PyExc_ValueError, "Expected a tuple of type 'i' in glib_bind_frame_buffer_object");
        return NULL;
    }
    glapi_BindFrameBufferObject(framebuffer);
    Py_RETURN_NONE;
}

static PyObject* glib_unbind_frame_buffer_object(PyObject* self, PyObject* args) {
    glapi_UnbindFrameBufferObject();
    Py_RETURN_NONE;
}

static PyObject* glib_depth_test(PyObject* self, PyObject* args) {
    int test;
    if (!PyArg_ParseTuple(args, "i", &test)) {
        PyErr_SetString(PyExc_ValueError, "Expected a bool in glib_depth_test");
        return NULL;
    }

    if (test) {
        glapi_EnableDepthTest();
    } else {
        glapi_DisableDepthTest();
    }

    Py_RETURN_NONE;
}

static PyObject* glib_get_window_width(PyObject* self, PyObject* args) {
    PyObject* app_capsule;
    if (!PyArg_ParseTuple(args, "O", &app_capsule)) {
        return NULL;
    }

    gl_app* app = (gl_app*)PyCapsule_GetPointer(app_capsule, "gl_app");
    if (!app) {
        PyErr_SetString(PyExc_ValueError, "Invalid gl_app pointer");
        return NULL;
    }

    return PyFloat_FromDouble((double)app->window->window_width);
}  

static PyObject* glib_get_window_height(PyObject* self, PyObject* args) {
    PyObject* app_capsule;
    if (!PyArg_ParseTuple(args, "O", &app_capsule)) {
        return NULL;
    }

    gl_app* app = (gl_app*)PyCapsule_GetPointer(app_capsule, "gl_app");
    if (!app) {
        PyErr_SetString(PyExc_ValueError, "Invalid gl_app pointer");
        return NULL;
    }

    return PyFloat_FromDouble((double)app->window->window_height);
}

static PyObject* glib_degs_to_rads(PyObject* self, PyObject* args) {
    float val;
    if (!PyArg_ParseTuple(args, "f", &val)) {
        PyErr_SetString(PyExc_TypeError, "Expected a float value to convert");
        return NULL;
    }

    return PyFloat_FromDouble((double)degs_to_rads(val));
}

static PyObject* glib_rads_to_degs(PyObject* self, PyObject* args) {
    float val;
    if (!PyArg_ParseTuple(args, "f", &val)) {
        PyErr_SetString(PyExc_TypeError, "Expected a float value to convert");
        return NULL;
    }

    return PyFloat_FromDouble((double)rads_to_degs(val));
}

static PyObject* glib_create_app(PyObject* self, PyObject* args) {
    uint16_t window_width, window_height;
    const char* title;
    int resizable_int;
    float r, g, b;

    if (!PyArg_ParseTuple(args, "HHspfff", &window_width, &window_height, &title, &resizable_int, &r, &g, &b)) {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments for create_app");
        return NULL;
    }

    bool resizable = (bool)resizable_int;
    gl_app* app = glapi_CreateApp(window_width, window_height, title, resizable, r, g, b);
    if (!app) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to create gl_app");
        return NULL;
    }

    return PyCapsule_New(app, "gl_app", NULL);
}

static PyObject* glib_bind_app(PyObject* self, PyObject* args) {
    PyObject* app_capsule;
    if (!PyArg_ParseTuple(args, "O", &app_capsule)) {
        return NULL;
    }

    gl_app* app = (gl_app*)PyCapsule_GetPointer(app_capsule, "gl_app");
    if (!app) {
        PyErr_SetString(PyExc_ValueError, "Invalid gl_app pointer");
        return NULL;
    }

    glapi_BindApp(app);
    Py_RETURN_NONE;
}

static PyObject* glib_unbind_app(PyObject* self, PyObject* args) {
    PyObject* app_capsule;
    if (!PyArg_ParseTuple(args, "O", &app_capsule)) {
        return NULL;
    }

    gl_app* app = (gl_app*)PyCapsule_GetPointer(app_capsule, "gl_app");
    if (!app) {
        PyErr_SetString(PyExc_ValueError, "Invalid gl_app pointer");
        return NULL;
    }

    glapi_UnbindApp(app);
    Py_RETURN_NONE;
}

static PyObject* glib_destroy_app(PyObject* self, PyObject* args) {
    PyObject* app_capsule;
    if (!PyArg_ParseTuple(args, "O", &app_capsule)) {
        return NULL;
    }

    gl_app* app = (gl_app*)PyCapsule_GetPointer(app_capsule, "gl_app");
    if (!app) {
        PyErr_SetString(PyExc_ValueError, "Invalid gl_app pointer");
        return NULL;
    }

    glapi_DestroyApp(app);
    Py_RETURN_NONE;
}

static PyObject* glib_should_app_close(PyObject* self, PyObject* args) {
    PyObject* app_capsule;
    if (!PyArg_ParseTuple(args, "O", &app_capsule)) {
        return NULL;
    }

    gl_app* app = (gl_app*)PyCapsule_GetPointer(app_capsule, "gl_app");
    if (!app) {
        PyErr_SetString(PyExc_ValueError, "Invalid gl_app pointer");
        return NULL;
    }

    int should_close = glapi_ShouldAppClose(app);
    return PyBool_FromLong(should_close);
}

static PyObject* glib_bind_vertex_buffer_object(PyObject* self, PyObject* args) {
    GLuint vao;
    if (!PyArg_ParseTuple(args, "I", &vao)) {
        return NULL;
    }

    glapi_BindVertexBufferObject(vao);
    Py_RETURN_NONE;
}

static PyObject* glib_unbind_vertex_buffer_object(PyObject* self, PyObject* args) {
    glapi_UnbindVertexBufferObject();
    Py_RETURN_NONE;
}

static PyObject* glib_bind_shader(PyObject* self, PyObject* args) {
    GLuint shader;
    if (!PyArg_ParseTuple(args, "I", &shader)) {
        return NULL;
    }

    glapi_BindShader(shader);
    Py_RETURN_NONE;
}

static PyObject* glib_unbind_shader(PyObject* self, PyObject* args) {
    glapi_UnbindShader();
    Py_RETURN_NONE;
}

static PyObject* glib_draw_vertex_buffer_object(PyObject* self, PyObject* args) {
    Py_ssize_t index_count;
    if (!PyArg_ParseTuple(args, "n", &index_count)) {
        return NULL;
    }

    glapi_DrawVertexBufferObject((size_t)index_count);
    Py_RETURN_NONE;
}

static PyObject* glib_gen_shader_program_f(PyObject* self, PyObject* args) {
    PyObject* app_capsule;
    const char* v_fpath;
    const char* f_fpath;
    if (!PyArg_ParseTuple(args, "Oss", &app_capsule, &v_fpath, &f_fpath)) {
        return NULL;
    }

    gl_app* app = (gl_app*)PyCapsule_GetPointer(app_capsule, "gl_app");
    if (!app) {
        PyErr_SetString(PyExc_ValueError, "Invalid gl_app pointer");
        return NULL;
    }

    GLuint shader;
    shader = glapi_GenShaderProgram_f(app, v_fpath, f_fpath, &shader);
    if (!shader) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to create shader program");
        return NULL;
    }

    return PyLong_FromUnsignedLong(shader);
}

static PyObject* glib_gen_shader_program_s(PyObject* self, PyObject* args) {
    PyObject* app_capsule;
    const char* v_source;
    const char* f_source;
    if (!PyArg_ParseTuple(args, "Oss", &app_capsule, &v_source, &f_source)) {
        return NULL;
    }

    gl_app* app = (gl_app*)PyCapsule_GetPointer(app_capsule, "gl_app");
    if (!app) {
        PyErr_SetString(PyExc_ValueError, "Invalid gl_app pointer");
        return NULL;
    }

    GLuint shader;
    shader = glapi_GenShaderProgram_s(app, v_source, f_source, &shader);
    if (!shader) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to create shader program");
        return NULL;
    }

    return PyLong_FromUnsignedLong(shader);
}

static PyObject* glib_gen_texture_from_fpath(PyObject* self, PyObject* args) {
    PyObject* app_capsule;
    const char* fpath;
    if (!PyArg_ParseTuple(args, "Os", &app_capsule, &fpath)) {
        return NULL;
    }

    gl_app* app = (gl_app*)PyCapsule_GetPointer(app_capsule, "gl_app");
    if (!app) {
        PyErr_SetString(PyExc_ValueError, "Invalid gl_app pointer");
        return NULL;
    }

    GLuint texture;
    texture = glapi_GenTextureFromFpath(app, fpath, &texture);
    if (!texture) {
        PyErr_SetString(PyExc_IOError, "Failed to load texture");
        return NULL;
    }

    return PyLong_FromUnsignedLong(texture);
}

static PyObject* glib_push_int_to_shader(PyObject* self, PyObject* args) {
    const char* varname;
    int value;
    GLuint shader;
    if (!PyArg_ParseTuple(args, "siI", &varname, &value, &shader)) {
        return NULL;
    }

    glapi_PushIntToShader(varname, value, shader);
    Py_RETURN_NONE;
}

static PyObject* glib_push_float_to_shader(PyObject* self, PyObject* args) {
    const char* varname;
    float value;
    GLuint shader;
    if (!PyArg_ParseTuple(args, "sfI", &varname, &value, &shader)) {
        return NULL;
    }

    glapi_PushFloatToShader(varname, value, shader);
    Py_RETURN_NONE;
}

static PyObject* glib_push_vec2_to_shader(PyObject* self, PyObject* args) {
    const char* varname;
    PyObject* vec2_obj;
    GLuint shader;
    float value[2];

    if (!PyArg_ParseTuple(args, "sOI", &varname, &vec2_obj, &shader)) {
        return NULL;
    }

    if (!sequence_to_float_array(vec2_obj, value, 2)) {
        return NULL;
    }

    glapi_PushVec2ToShader(varname, value, shader);
    Py_RETURN_NONE;
}

static PyObject* glib_push_vec3_to_shader(PyObject* self, PyObject* args) {
    const char* varname;
    PyObject* vec3_obj;
    GLuint shader;
    float value[3];

    if (!PyArg_ParseTuple(args, "sOI", &varname, &vec3_obj, &shader)) {
        return NULL;
    }

    if (!sequence_to_float_array(vec3_obj, value, 3)) {
        return NULL;
    }

    glapi_PushVec3ToShader(varname, value, shader);
    Py_RETURN_NONE;
}

static PyObject* glib_push_vec4_to_shader(PyObject* self, PyObject* args) {
    const char* varname;
    PyObject* vec4_obj;
    GLuint shader;
    float value[4];

    if (!PyArg_ParseTuple(args, "sOI", &varname, &vec4_obj, &shader)) {
        return NULL;
    }

    if (!sequence_to_float_array(vec4_obj, value, 4)) {
        return NULL;
    }

    glapi_PushVec4ToShader(varname, value, shader);
    Py_RETURN_NONE;
}

static PyObject* glib_push_matrix3x3_to_shader(PyObject* self, PyObject* args) {
    const char* varname;
    PyObject* matrix_obj;
    GLuint shader;
    float value[9];

    if (!PyArg_ParseTuple(args, "sOI", &varname, &matrix_obj, &shader)) {
        return NULL;
    }

    if (!sequence_to_float_array(matrix_obj, value, 9)) {
        return NULL;
    }

    glapi_PushMatrix3x3ToShader(varname, value, shader);
    Py_RETURN_NONE;
}

static PyObject* glib_push_matrix4x4_to_shader(PyObject* self, PyObject* args) {
    const char* varname;
    PyObject* matrix_obj;
    GLuint shader;
    float value[16];

    if (!PyArg_ParseTuple(args, "sOI", &varname, &matrix_obj, &shader)) {
        return NULL;
    }

    if (!sequence_to_float_array(matrix_obj, value, 16)) {
        return NULL;
    }

    glapi_PushMatrix4x4ToShader(varname, value, shader);
    Py_RETURN_NONE;
}

static PyObject* glib_push_texture2D_to_shader(PyObject* self, PyObject* args) {
    const char* varname;
    GLuint texture;
    GLuint shader;
    if (!PyArg_ParseTuple(args, "sII", &varname, &texture, &shader)) {
        return NULL;
    }

    glapi_PushTexture2DToShader(varname, texture, shader);
    Py_RETURN_NONE;
}

static PyMethodDef GLIBMethods[] = {
    {"depth_test", glib_depth_test, METH_VARARGS, "Enable/Disable depth testing"},
    {"get_window_width", glib_get_window_width, METH_VARARGS, "Get window width in px from gl_app object"},
    {"get_window_height", glib_get_window_height, METH_VARARGS, "Get window height in px from gl_app object"},
    {"degs_to_rads", glib_degs_to_rads, METH_VARARGS, "Convert degrees to radians"},
    {"rads_to_degs", glib_rads_to_degs, METH_VARARGS, "Convert radians to degrees"},
    {"vec2_add", glib_vec2_add, METH_VARARGS, "Add two vec2 objects and return a result"},
    {"vec3_add", glib_vec3_add, METH_VARARGS, "Add two vec3 objects and return a result"},
    {"vec4_add", glib_vec4_add, METH_VARARGS, "Add two vec4 objects and return a result"},
    {"matrix4x4_multi", glib_matrix4x4_multi, METH_VARARGS, "Multiply two matrix4x4 objects and return a result"},
    {"transform_matrix4x4", glib_transform_matrix4x4, METH_VARARGS, "Transform a matrix4x4 to a position, rotation, and scale"},
    {"view_matrix4x4", glib_view_matrix4x4, METH_VARARGS, "Transform a matrix4x4 to a cameras position and rotation"},
    {"projection_matrix4x4", glib_projection_matrix4x4, METH_VARARGS, "Transform a matrix4x4 to a cameras projection"},
    {"create_app", glib_create_app, METH_VARARGS, "Create an OpenGL application window"},
    {"bind_app", glib_bind_app, METH_VARARGS, "Bind the application's context"},
    {"unbind_app", glib_unbind_app, METH_VARARGS, "Unbind the application's context"},
    {"destroy_app", glib_destroy_app, METH_VARARGS, "Destroy the OpenGL application"},
    {"should_app_close", glib_should_app_close, METH_VARARGS, "Check if the application should close"},
    {"bind_vertex_buffer_object", glib_bind_vertex_buffer_object, METH_VARARGS, "Bind a vertex buffer object"},
    {"unbind_vertex_buffer_object", glib_unbind_vertex_buffer_object, METH_VARARGS, "Unbind a vertex buffer object"},
    {"bind_frame_buffer_object", glib_bind_frame_buffer_object, METH_VARARGS, "Bind a frame buffer object"},
    {"unbind_frame_buffer_object", glib_unbind_frame_buffer_object, METH_VARARGS, "Unbind a frame buffer object"},
    {"bind_shader", glib_bind_shader, METH_VARARGS, "Bind a shader program"},
    {"unbind_shader", glib_unbind_shader, METH_VARARGS, "Unbind a shader program"},
    {"draw_vertex_buffer_object", glib_draw_vertex_buffer_object, METH_VARARGS, "Draw a vertex buffer object"},
    {"gen_shader_program_f", glib_gen_shader_program_f, METH_VARARGS, "Generate shader program from file paths"},
    {"gen_shader_program_s", glib_gen_shader_program_s, METH_VARARGS, "Generate shader program from source"},
    {"gen_vertex_buffer_object", glib_gen_vertex_buffer_object, METH_VARARGS, "Generate vertex buffer object from mesh"},
    {"gen_frame_buffer_object", glib_gen_frame_buffer_object, METH_VARARGS, "Generate frame buffer object"},
    {"gen_texture_from_fpath", glib_gen_texture_from_fpath, METH_VARARGS, "Generate texture from file path"},
    {"push_int_to_shader", glib_push_int_to_shader, METH_VARARGS, "Push integer to shader uniform"},
    {"push_float_to_shader", glib_push_float_to_shader, METH_VARARGS, "Push float to shader uniform"},
    {"push_vec2_to_shader", glib_push_vec2_to_shader, METH_VARARGS, "Push vec2 to shader uniform"},
    {"push_vec3_to_shader", glib_push_vec3_to_shader, METH_VARARGS, "Push vec3 to shader uniform"},
    {"push_vec4_to_shader", glib_push_vec4_to_shader, METH_VARARGS, "Push vec4 to shader uniform"},
    {"push_matrix3x3_to_shader", glib_push_matrix3x3_to_shader, METH_VARARGS, "Push 3x3 matrix to shader uniform"},
    {"push_matrix4x4_to_shader", glib_push_matrix4x4_to_shader, METH_VARARGS, "Push 4x4 matrix to shader uniform"},
    {"push_texture2D_to_shader", glib_push_texture2D_to_shader, METH_VARARGS, "Push 2D texture to shader uniform"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef glibmodule = {
    PyModuleDef_HEAD_INIT,
    "glib",
    "Python interface for OpenGL graphics API",
    -1,
    GLIBMethods
};

PyMODINIT_FUNC PyInit_glib(void) {
    return PyModule_Create(&glibmodule);
}