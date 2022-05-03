#version 330

// #extension GL_ARB_texture_rectangle : enable
#extension GL_ARB_draw_buffers : enable

uniform sampler2DRect u_texture0;
uniform sampler2DRect u_depth_texture;

uniform vec3 u_sphere_pos;
uniform float u_sphere_radius;
uniform float u_div;

in vec2 vTextureCoord; // 330
layout(location = 0) out vec4 out_frag; // 330

void main(void) {
    // vec2 st = gl_MultiTexCoord0.xy;
    // vec2 st = gl_TexCoord[0].st;
    vec4 pos = texture(u_texture0, vTextureCoord);
    
    if (pos.w == 0.0) {
        out_frag = pos;
        return;
    }

    if (
        pos.x < -1.0 ||
        pos.x > 0.99 || // now hre... hmmm
        pos.y < -1.0 ||
        pos.y > 1.0 // now here.... hmmm
    ) {
        out_frag = pos;
        return;
    }

    vec2 st_depth;
    st_depth.x = (pos.x + 1.0) * 0.5 * 576.0;
    // st_depth.y = (pos.y + 1.0) * 0.5 * 576.0;
    st_depth.y = (pos.y + 1.0) * 0.5 * 432.0;

    float depth = 1.0 - texture(u_depth_texture, st_depth).r;

    if (depth < 0.0) {
        out_frag = pos;
        return;
    }

    if (depth < pos.z) {
        out_frag = pos;
        return;
    }

    // gl_FragData[0].rgba = vec4(pos.x, pos.y, depth*1.0, pos.w);
    out_frag = vec4(pos.x, pos.y, depth, pos.w);
    return;

}