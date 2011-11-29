// Just composites together the background layers

uniform sampler2D u_colormap0;
uniform sampler2D u_colormap1;
uniform sampler2D u_colormap2;
uniform sampler2D u_colormap3;

varying vec2 v_texCoord0;
varying vec2 v_texCoord1;
varying vec2 v_texCoord2;
varying vec2 v_texCoord3;

void main()
{
	vec4 result = texture2D(u_colormap0, v_texCoord0);
	vec4 temp;
	temp = texture2D(u_colormap1, v_texCoord1);
	result = mix(result, temp, temp.a);

	temp = texture2D(u_colormap2, v_texCoord2);
	result = mix(result, temp, temp.a);

	temp = texture2D(u_colormap3, v_texCoord3);
	result = mix(result, temp, temp.a);

	gl_FragColor = result;
}
