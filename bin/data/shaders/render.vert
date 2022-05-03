#version 330
precision mediump float;

// #extension GL_EXT_gpu_shader4 : enable
// #extension GL_ARB_texture_rectangle : enable

uniform mat4 modelViewProjectionMatrix;

uniform float u_div;
uniform float u_neightbor_offset;
uniform float u_face;
uniform mat4 u_matrix_for_normal;
uniform sampler2DRect u_posAndAgeTex;

in vec4 position;
in vec2 texcoord;

out vec3 v_wpos;
out vec3 v_normal;
out vec2 v_st;

void main() {
  // vec2 st = gl_MultiTexCoord0.xy;
  vec3 pos = texture2DRect(u_posAndAgeTex, texcoord).xyz; //位置を取得

  float offset_x = (texcoord.x/u_div) + u_neightbor_offset < 1.0 ? u_neightbor_offset : -u_neightbor_offset;
	float offset_y = (texcoord.y/u_div) + u_neightbor_offset < 1.0 ? u_neightbor_offset : -u_neightbor_offset;

	vec3 pos1 = texture2DRect(u_posAndAgeTex, texcoord).xyz;
	vec3 pos2 = texture2DRect(u_posAndAgeTex, texcoord + vec2(offset_x*u_div, 0.0)).xyz;
	vec3 pos3 = texture2DRect(u_posAndAgeTex, texcoord + vec2(0.0, offset_y*u_div)).xyz;

	vec3 dir1 = pos2 - pos1;
	vec3 dir2 = pos3 - pos1;

	vec3 n = (offset_x * offset_y > 0.0 ? cross(dir1, dir2) : cross(dir2, dir1)) * u_face;
	n = normalize(n);

	v_wpos = pos1;
	v_normal = mat3(u_matrix_for_normal) * n;
	v_st = texcoord;

  gl_Position = modelViewProjectionMatrix * vec4(pos1 * 400.0, 1.0); //パーティクルの位置を設定
}
