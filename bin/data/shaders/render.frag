#version 330
precision mediump float;

// #extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect u_posAndAgeTex;
uniform sampler2DRect u_imageTex;
uniform vec3 u_directional_light_dir;
uniform vec3 u_ambient_light_color;
uniform vec3 u_directional_light_color;
uniform vec3 u_specular_light_color;

uniform vec3 u_camera_pos;

in vec3 v_wpos;
in vec3 v_normal;
in vec2 v_st;

out vec4 outputColor;

void main() {
	// texRes = 100. width:100, height:100
	// width:576, height:432
	vec2 v_st_post;

    v_st_post.x = (v_wpos.x + 1.0) * 0.5 * 576.0;
    // v_st_post.y = (v_wpos.y + 1.0) * 0.5 * 576.0;
    v_st_post.y = (v_wpos.y + 1.0) * 0.5 * 432.0;

    vec3 color = texture2DRect(u_imageTex, v_st_post).xyz + u_specular_light_color;


    // Lighting
    // vec3 color = u_specular_light_color;
	vec3 normal = normalize(v_normal);

	vec3 ambient_light_color = u_ambient_light_color;

	vec3 directional_light_dir = normalize(u_directional_light_dir);
	vec3 directional_light_color = u_directional_light_color;

	vec3 ray_dir = normalize(v_wpos.xyz - u_camera_pos);

	vec3 half_vec = normalize(-directional_light_dir - ray_dir);

	color *= ambient_light_color + max(dot(normal, -directional_light_dir), 0.0) * directional_light_color;
	color += pow(max(dot(normal, half_vec), 0.0), 10.0) * vec3(0.35, 0.35, 0.1);
	color += pow(1.0 - max(dot(-ray_dir, normal), 0.0), 2.75) * vec3(0.45, 0.45, 0.45);

	// gl_FragColor = vec4(color, 1.0);
	outputColor = vec4(color, 1.0);
    // gl_FragColor = gl_Color;
}
