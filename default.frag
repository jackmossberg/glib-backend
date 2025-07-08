#version 330 core
out vec4 final_color;
in vec3 i_position;
in vec3 i_normal;
in vec2 i_uv;

in vec3 i_gnormal;
in vec3 i_gposition;

uniform float tex0_scale;
uniform sampler2D tex0;

#define MAX_LIGHTS 132
uniform vec3 point_position[MAX_LIGHTS];
uniform vec3 point_color[MAX_LIGHTS];
uniform float point_strength[MAX_LIGHTS];
uniform float point_atten[MAX_LIGHTS];

uniform float ambient_light_pow;

void main() {
    vec3 light_col = vec3(0.0);
    float light_v = 0.0;

    for (int i = 0; i < MAX_LIGHTS; i++) {
        vec3 to_light = point_position[i] - i_gposition;
        float dist = length(to_light);
        vec3 light_dir = normalize(to_light);
        float diff = max(dot(normalize(i_gnormal), light_dir), 0.0);
        float atten = point_strength[i] / (1.0 + point_atten[i] * dist * dist);
        light_v += diff * atten;
        light_col += point_color[i] * diff * atten;
    }
    light_v = clamp(light_v, 0.0, 1.0);

    vec4 tex_color = texture(tex0, i_uv * tex0_scale);
    vec3 ambient = tex_color.rgb * ambient_light_pow;
    vec3 diffuse = tex_color.rgb * light_col;
    final_color = vec4(ambient + diffuse * light_v, tex_color.a);
}