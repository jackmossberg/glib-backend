#version 330 core
out vec4 final_color;
in vec3 i_position;
in vec3 i_normal;
in vec2 i_uv;
uniform sampler2D tex0;
void main() {
    //final_color = vec4(i_position.x, i_position.y, i_position.z, 1.0f);
    final_color = texture(tex0, i_uv) * vec4(1.25);

    if (final_color.x < 0.3) {
        final_color = vec4(0.0);
    }

    final_color *= vec4(0.5);
}