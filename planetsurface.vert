#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;

out vec3 i_position;
out vec3 i_normal;
out vec2 i_uv;

out vec3 i_gnormal;
out vec3 i_gposition;

uniform mat4 transform_matrix;
uniform mat4 projection_matrix;
uniform mat4 view_matrix;

void main() {
    i_position = position;
    i_normal = normal;
    i_uv = uv;

    i_gnormal = normalize(mat3(transform_matrix) * i_normal);
    vec4 i_gtransform = transform_matrix * vec4(position, 1.0f);
    i_gposition = i_gtransform.xyz;

    gl_Position = projection_matrix * view_matrix * transform_matrix * vec4(position, 1.0f);
    //gl_Position = transform_matrix * vec4(position, 1.0f);
    //gl_Position = vec4(position, 1.0f);
}