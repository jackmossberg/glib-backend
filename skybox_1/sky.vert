#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;
out vec3 i_position;
out vec3 i_normal;
out vec2 i_uv;
uniform mat4 transform_matrix;
uniform mat4 projection_matrix;
uniform mat4 view_matrix;
void main() {
    i_position = position;
    i_normal = normal;
    i_uv = uv;
    gl_Position = projection_matrix * view_matrix * transform_matrix * vec4(position, 1.0f);
    //gl_Position = transform_matrix * vec4(position, 1.0f);
    //gl_Position = vec4(position, 1.0f);
}