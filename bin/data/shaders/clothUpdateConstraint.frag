#version 330

// #extension GL_ARB_texture_rectangle : enable
// #extension GL_ARB_draw_buffers : enable

uniform sampler2DRect u_texture0;
uniform float u_div;
uniform vec2 u_neightbor_offset;
uniform float u_neightbor_coefficient;
uniform float u_rest;
uniform float u_k;
uniform float u_shrink;
uniform float u_stretch;
uniform float u_f2dx;

in vec2 vTextureCoord;

layout(location = 0) out vec4 out_frag;

vec2 neightbor(vec2 st)
{
	vec2 of = u_neightbor_offset;
	// vec2 stp = step(0.5, mod(st * (u_div + 1.0) / u_neightbor_coefficient, 1.0));
	// vec2 stp = step(0.5, mod(st * (u_div - 1.0) / u_neightbor_coefficient, 1.0));
	vec2 stp = step(0.5, mod(st * u_div / u_neightbor_coefficient, 1.0));
	of = of * stp - of * (1.0 - stp);
	return st + of;
}

void main(void) {
	vec4 pos1 = texture(u_texture0, vTextureCoord);

	// gl_FragData[0].rgba = pos1;//Debug
	// gl_FragData[0].rgba = vec4((st.x/u_div), (st.y/u_div), 0.0, 1.0);//Debug
	// return;
	
	if (pos1.w == 0.0) {
		out_frag = pos1;
		// gl_FragData[0].rgba = vec4(1.0, 0.0, 0.0, 1.0);//Debug
		return;
	}

	vec2 uv_neightbor = neightbor(vTextureCoord/u_div);
	// vec2 uv_neightbor = st + u_neightbor_offset;
	// vec2 uv_neightbor = st + vec2(0.010101, 0.0);

	if (
		uv_neightbor.x < 0.0 ||
		uv_neightbor.x > 1.0 || // now hre... hmmm
		uv_neightbor.y < 0.0 ||
		uv_neightbor.y > 1.0 // now here.... hmmm
	) {
		out_frag = pos1;
		// gl_FragData[0].rgba = vec4(0.0, 1.0, 0.0, 1.0);//Debug
		// gl_FragData[0].rgba = vec4(uv_neightbor.x, uv_neightbor.y, 0.0, 1.0);//Debug
		return;
	}

	// gl_FragData[0].rgba = vec4(0.0, 0.0, 1.0, 1.0);//Debug
	// return;//Debug

	vec4 pos2 = texture2DRect(u_texture0, uv_neightbor*u_div);
	// vec4 pos2 = vec4(st.x + 0.0101, st.y + 0.0101, 0.0101, 1.0);
	
	float d = length(pos2.xyz - pos1.xyz);
	float f = (d - u_rest) * u_k;
	float stp = step(0.0, f);
	f *= u_shrink * stp + u_stretch * (1.0 - stp);
	vec3 dx = normalize(pos2.xyz - pos1.xyz) * f * u_f2dx;

	// pos1.xyz += dx;
	pos1.xyz += dx * pos1.w / (pos1.w + pos2.w);

	out_frag = pos1;
}