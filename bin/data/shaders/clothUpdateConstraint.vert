#version 330
precision mediump float;

// uniform mat4 model;
// uniform mat4 view;
// uniform mat4 projection;
// uniform mat4 mvp;
uniform mat4 modelViewProjectionMatrix;

in vec2 texcoord;
in vec3 position;
// in vec4 color;

out vec2 vTextureCoord;
// out vec4 vColor;

void main() {
    vTextureCoord = texcoord;
    // vColor = color;
    //gl_Position = projection * view * model * vec4(position, 1.0);
    // gl_Position = mvp * vec4(position, 1.0);
    gl_Position = modelViewProjectionMatrix * vec4(position, 1.0);
}
