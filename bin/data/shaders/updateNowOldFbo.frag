#version 330

// #extension GL_ARB_texture_rectangle : enable
// #extension GL_ARB_draw_buffers : enable

uniform sampler2DRect texture0;

in vec2 vTextureCoord;

layout(location = 0) out vec4 out_frag;

void main(void) {
    vec4 tex = texture2DRect(texture0, vTextureCoord);

    out_frag = tex;
}