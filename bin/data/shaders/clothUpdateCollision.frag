#version 330
precision mediump float;

uniform sampler2DRect u_texture0;

uniform vec3 u_sphere_pos;
uniform float u_sphere_radius;
uniform float u_div;

in vec2 vTextureCoord;

layout(location = 0) out vec4 out_frag;

void main(void) {
    vec4 pos = texture2DRect(u_texture0, vTextureCoord);
    vec3 v = pos.xyz - u_sphere_pos;
    float d = length(v);
    float stp = step(0.0, d - u_sphere_radius);

    pos.xyz =
        pos.xyz * stp
        + (u_sphere_pos + normalize(v) * u_sphere_radius)
        * (1.0 - stp);

    out_frag = pos;
}