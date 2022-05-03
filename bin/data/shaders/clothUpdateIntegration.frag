#version 330
precision mediump float;

// #extension GL_ARB_texture_rectangle : enable
// #extension GL_ARB_draw_buffers : enable

uniform sampler2DRect u_tex_now;
uniform sampler2DRect u_tex_old;

uniform vec3 u_dx;
uniform float u_r;

in vec2 vTextureCoord;
in vec4 vColor;
// varying vec3 pos;

layout(location = 0) out vec4 out_frag;

void main(void) {
    vec4 pos_now = texture2DRect(u_tex_now, vTextureCoord);
    vec4 pos_old = texture2DRect(u_tex_old, vTextureCoord);

    vec3 dx = pos_now.xyz - pos_old.xyz;

    dx += u_dx;
	dx *= u_r;
    dx *= pos_now.w;

    out_frag = vec4(pos_now.xyz + dx, pos_now.w);
}