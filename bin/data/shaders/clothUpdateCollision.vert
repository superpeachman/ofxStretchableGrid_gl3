#version 330
precision mediump float;

uniform mat4 modelViewProjectionMatrix;

in vec2 texcoord;
in vec3 position;

out vec2 vTextureCoord;

void main() {
    vTextureCoord = texcoord;
    gl_Position = modelViewProjectionMatrix * vec4(position, 1.0);
}
