#version 330
precision mediump float; // 330

// #extension GL_ARB_draw_buffers : enable // 120

uniform mat4 modelViewProjectionMatrix;
in vec2 texcoord;
in vec3 position;

out vec2 vTextureCoord;


void main() {
    vTextureCoord = texcoord;
    gl_Position = modelViewProjectionMatrix * vec4(position, 1.0);

    // texCoordVarying = gl_MultiTexCoord0.xy;
    // gl_Position = ftransform();
}